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

#include "AVVMUtils.h"
#include "AVVMWorldSetting.h"
#include "Batchable.h"
#include "BatchingRule.h"
#include "BatchSample.h"
#include "BatchSampleDeveloperSettings.h"
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
	if (!IsValid(WorldSettings) || !WorldSettings->ShouldCreatePluginRule(TAG_WORLD_RULE_BATCHING))
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
	Clear();
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

	TArray<FBatchContext> NewPendingDestroy = MoveTemp(PendingDestroy);
	for (auto Iterator = NewPendingDestroy.CreateIterator(); Iterator; ++Iterator)
	{
		const bool bDoesSupportTrashingUndersizeBatch = (!FMath::IsNearlyZero(MaxLifetimeAllowedToUndersizeBatch) && (MaxLifetimeAllowedToUndersizeBatch >= Interval));
		const bool bDoesElapseTimeQualify = (bDoesSupportTrashingUndersizeBatch && (Timestamp >= MaxLifetimeAllowedToUndersizeBatch));
		const bool bDoesQualify = (Iterator->DoesQualifyForBatchDestroy(MaxSizePerBatchDestroy) || bDoesElapseTimeQualify);
		if (bDoesQualify)
		{
			Iterator->Invalidate();
			bHasDestroyedBatched |= true;
		}
		else
		{
			PendingDestroy.Add(MoveTemp(*Iterator));
			Iterator.RemoveCurrentSwap();
		}
	}

	if (bHasDestroyedBatched)
	{
		Timestamp = 0.f;
	}

	if (bShouldGarbageOnNextTick)
	{
		GarbageOnNextTick(NewPendingDestroy);
	}
	else
	{
		GarbageNow(NewPendingDestroy);
	}
}

TStatId UBatchingSubsystem::GetStatId() const
{
	return TStatId();
}

void UBatchingSubsystem::Static_Unregister(const UWorld* World, AActor* Actor)
{
	auto* BatchingSubsystem = UBatchingSubsystem::Get(World);
	if (IsValid(BatchingSubsystem))
	{
		BatchingSubsystem->UnRegister(Actor);
	}
}

void UBatchingSubsystem::Static_Register(const UWorld* World, AActor* Actor)
{
	auto* BatchingSubsystem = UBatchingSubsystem::Get(World);
	if (IsValid(BatchingSubsystem))
	{
		BatchingSubsystem->Register(Actor);
	}
}

void UBatchingSubsystem::Static_Clear(const UWorld* World)
{
	auto* BatchingSubsystem = UBatchingSubsystem::Get(World);
	if (IsValid(BatchingSubsystem))
	{
		BatchingSubsystem->Clear();
	}
}

bool UBatchingSubsystem::Static_IsEmpty(const UWorld* World)
{
	auto* BatchingSubsystem = UBatchingSubsystem::Get(World);
	return IsValid(BatchingSubsystem) ? BatchingSubsystem->IsEmpty() : true;
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
	if (!UAVVMUtils::IsNativeScriptInterfaceValid(Batchable) || Batchable->IsPlacedInEditor())
	{
		return;
	}

	if (!ensureAlwaysMsgf(Batchable->HasValidBatchIndex(),
	                      TEXT("IBatchable doesnt reference a valid Batch index. Actor may have been marked for Destroy on Next tick.")))
	{
		return;
	}

	const int32 BatchIndex = Batchable->GetOwningBatchIndex();
	const bool bIsValidIndex = PendingDestroy.IsValidIndex(BatchIndex);
	if (bIsValidIndex)
	{
		PendingDestroy[BatchIndex].Remove(Actor);
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
	if (!UAVVMUtils::IsNativeScriptInterfaceValid(Batchable) || Batchable->IsPlacedInEditor())
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
			Top.Add(Actor);
		}
	}

	Batchable->SetOwningBatchIndex(PendingDestroy.Num());
}

void UBatchingSubsystem::Clear()
{
	StreamableHandle.Reset();
	PendingDestroy.Empty();
}

#if WITH_AUTOMATION_TESTS
bool UBatchingSubsystem::IsEmpty() const
{
	return PendingDestroy.IsEmpty() && !NextTickHandle.IsValid();
}
#endif

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
		StreamableHandle = WorldSettings->AsyncLoadPluginRule(UBatchSampleDeveloperSettings::GetBatchingRuleClass(), Callback);
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
		bShouldGarbageOnNextTick = Rule->ShouldGarbageOnNextTick();
	}
}

void UBatchingSubsystem::GarbageOnNextTick(TArray<FBatchContext>& NewPendingDestroy)
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const auto DeferredBatchAction = [](TArray<FBatchContext> NewPendingDestroy, FTimerHandle* TimerHandle)
	{
		for (int32 i = NewPendingDestroy.Num() - 1; i >= 0; --i)
		{
			NewPendingDestroy[i].Obliterate();
		}

		if (TimerHandle != nullptr)
		{
			TimerHandle->Invalidate();
		}
	};

	const auto Callback = FTimerDelegate::CreateWeakLambda(this, DeferredBatchAction, NewPendingDestroy, &NextTickHandle);
	NextTickHandle = World->GetTimerManager().SetTimerForNextTick(Callback);
}

void UBatchingSubsystem::GarbageNow(TArray<FBatchContext>& NewPendingDestroy) const
{
	for (int32 i = NewPendingDestroy.Num() - 1; i >= 0; --i)
	{
		NewPendingDestroy[i].Obliterate();
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
		if (!UAVVMUtils::IsNativeScriptInterfaceValid(Batchable))
		{
			Iterator.RemoveCurrentSwap();
		}
		else
		{
			(*Iterator)->Destroy();
		}
	}
}

void UBatchingSubsystem::FBatchContext::Add(AActor* Actor)
{
	Candidates.Add(Actor);
}

void UBatchingSubsystem::FBatchContext::Remove(AActor* Actor)
{
	Candidates.Remove(Actor);
}

void UBatchingSubsystem::FBatchContext::Invalidate() const
{
	for (auto Iterator = Candidates.CreateConstIterator(); Iterator; ++Iterator)
	{
		const auto Batchable = TScriptInterface<IBatchable>(Iterator->Get());
		if (UAVVMUtils::IsNativeScriptInterfaceValid(Batchable))
		{
			Batchable->SetOwningBatchIndex(INDEX_NONE);
		}
	}
}
