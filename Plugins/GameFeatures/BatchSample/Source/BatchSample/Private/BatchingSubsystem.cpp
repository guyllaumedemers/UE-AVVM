//Copyright(c) 2025 gdemers
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
#include "BatchingSubsystem.h"

#include "AVVMUtilityFunctionLibrary.h"
#include "AVVMWorldSetting.h"
#include "Batchable.h"
#include "BatchingRule.h"
#include "BatchSample.h"
#include "NativeGameplayTags.h"
#include "Engine/StreamableManager.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_WORLD_RULE_BATCHING, "WorldRule.Batching");

bool UBatchingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (!IsValid(World))
	{
		return false;
	}

	bool bHasAuthority = !World->IsNetMode(NM_Client);

	const auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (!IsValid(WorldSettings) || !WorldSettings->ShouldCreateRule(TAG_WORLD_RULE_BATCHING))
	{
		return false;
	}

	if (bHasAuthority)
	{
		bHasAuthority |= FBatchSampleModule::GetCVarBatchEnableSubsystem()->GetBool();
	}

	return bHasAuthority;
}

void UBatchingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CreateBatchingRule();
}

void UBatchingSubsystem::Deinitialize()
{
	Super::Deinitialize();
	PendingDestroy.Empty();
}

void UBatchingSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PendingDestroy.IsEmpty())
	{
		return;
	}

	Timestamp = FMath::Clamp(Timestamp + DeltaTime, 0.f, MaxLifetimeAllowedToUndersizeBatch);
	if (Timestamp < Interval)
	{
		return;
	}

	bool bHasDestroyedBatched = false;

	TArray<FBatchContext> ImmutableBatches = MoveTemp(PendingDestroy);
	while (!ImmutableBatches.IsEmpty())
	{
		FBatchContext& BatchContext = ImmutableBatches[0];

		const bool bDoesSupportTrashingUndersizeBatch = (!FMath::IsNearlyZero(MaxLifetimeAllowedToUndersizeBatch) && (MaxLifetimeAllowedToUndersizeBatch >= Interval));
		const bool bDoesElapseTimeQualify = (bDoesSupportTrashingUndersizeBatch && (Timestamp >= MaxLifetimeAllowedToUndersizeBatch));
		const bool bDoesQualify = (BatchContext.DoesQualifyForBatchDestroy(MaxSizePerBatchDestroy) || bDoesElapseTimeQualify);
		if (bDoesQualify)
		{
			BatchContext.Obliterate();
			bHasDestroyedBatched |= true;
		}
		else
		{
			PendingDestroy.Add(MoveTemp(BatchContext));
		}

		ImmutableBatches.RemoveAtSwap(0);
	}

	if (bHasDestroyedBatched)
	{
		Timestamp = 0.f;
	}
}

TStatId UBatchingSubsystem::GetStatId() const
{
	return TStatId();
}

UBatchingSubsystem* UBatchingSubsystem::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UBatchingSubsystem>(World);
}

void UBatchingSubsystem::UnRegister(AActor* Actor)
{
	const UBatchingRule* Rule = BatchingRule.Get();
	if (!IsValid(Rule) || !Rule->DoesQualifyForBatchDestroy(Actor))
	{
		return;
	}

	const auto Batchable = TScriptInterface<IBatchable>(Actor);
	if (!UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(Batchable) || Batchable->IsMarkIgnored())
	{
		return;
	}

	const int32 BatchIndex = Batchable->GetBatchIndex();
	const bool bIsValidIndex = PendingDestroy.IsValidIndex(BatchIndex);
	if (bIsValidIndex)
	{
		PendingDestroy[BatchIndex].ForceRemove(Actor);
	}
}

void UBatchingSubsystem::Register(AActor* Actor)
{
	const UBatchingRule* Rule = BatchingRule.Get();
	if (!IsValid(Rule) || !Rule->DoesQualifyForBatchDestroy(Actor))
	{
		return;
	}

	const auto Batchable = TScriptInterface<IBatchable>(Actor);
	if (!UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(Batchable) || Batchable->IsMarkIgnored())
	{
		return;
	}

	if (PendingDestroy.IsEmpty())
	{
		PendingDestroy.Add(FBatchContext{Actor, MaxSizePerBatchDestroy});
	}
	else
	{
		FBatchContext& Top = PendingDestroy.Top();
		if (Top.DoesQualifyForBatchDestroy(MaxSizePerBatchDestroy))
		{
			PendingDestroy.Add(FBatchContext{Actor, MaxSizePerBatchDestroy});
		}
		else
		{
			Top.Push(Actor);
		}
	}

	Batchable->SetBatchIndex(PendingDestroy.Num());
}

void UBatchingSubsystem::Clear()
{
	PendingDestroy.Empty();
}

void UBatchingSubsystem::CreateBatchingRule()
{
	const auto OnAsyncLoadComplete = [](const TWeakObjectPtr<UBatchingSubsystem>& NewBatchingSubsystem,
	                                    const TWeakObjectPtr<AAVVMWorldSetting>& NewWorldSettings)
	{
		if (!NewBatchingSubsystem.IsValid() || !NewWorldSettings.IsValid())
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = NewBatchingSubsystem->StreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		const auto* RuleClass = Cast<UClass>(Handle->GetLoadedAsset());
		if (!IsValid(RuleClass))
		{
			return;
		}

		NewBatchingSubsystem->BatchingRule = NewWorldSettings->GetOrCreatePluginRule<UBatchingRule>(TAG_WORLD_RULE_BATCHING, RuleClass);
		NewBatchingSubsystem->InitRule();
	};

	const auto* World = GetTypedOuter<UWorld>();
	if (!IsValid(World))
	{
		return;
	}

	auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (IsValid(WorldSettings))
	{
		const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncLoadComplete, TWeakObjectPtr(this), TWeakObjectPtr(WorldSettings));
		StreamableHandle = WorldSettings->AsyncLoadPluginRule(UBatchingRule::StaticClass(), Callback);
	}
}

void UBatchingSubsystem::InitRule()
{
	const UBatchingRule* Rule = BatchingRule.Get();
	if (IsValid(Rule))
	{
		MaxLifetimeAllowedToUndersizeBatch = Rule->GetMaxLifetimeAllowedToUndersizeBatch();
		MaxSizePerBatchDestroy = Rule->GetMaxSizePerBatchDestroy();
		Interval = Rule->GetBatchInterval();
	}
}

bool UBatchingSubsystem::FBatchContext::DoesQualifyForBatchDestroy(const float MaxSize) const
{
	return Candidates.Num() >= MaxSize;
}

void UBatchingSubsystem::FBatchContext::Obliterate()
{
	for (auto Iterator = Candidates.CreateIterator(); Iterator; ++Iterator)
	{
		const auto Batchable = TScriptInterface<IBatchable>(Iterator->Get());
		if (!UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(Batchable) || Batchable->IsMarkIgnored())
		{
			Iterator.RemoveCurrentSwap();
		}
		else
		{
			(*Iterator)->Destroy();
		}
	}
}

void UBatchingSubsystem::FBatchContext::Push(AActor* Actor)
{
	Candidates.Add(Actor);
}

void UBatchingSubsystem::FBatchContext::ForceRemove(AActor* Actor)
{
	Candidates.Add(Actor);
}
