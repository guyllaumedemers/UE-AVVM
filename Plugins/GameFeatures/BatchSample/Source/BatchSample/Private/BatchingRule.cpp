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

#include "BatchingRule.h"

#include "Engine/AssetManager.h"

#if WITH_EDITOR
EDataValidationResult UBatchingRule::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (AllowedClasses.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UBatchingRule", "", "Classes is empty. You should remove this Rule to prevent Subsystem creation."));
	}

	return Result;
}
#endif

void UBatchingRule::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsTemplate())
	{
		return;
	}

	AllowedClasses_StreamableHandle.Reset();
	IgnoredClasses_StreamableHandle.Reset();
	AllowedActorClasses.Reset();
	IgnoredActorClasses.Reset();

	TArray<FSoftObjectPath> AllowedClasses_SoftObjectPaths;
	for (const auto& ImplClass : AllowedClasses)
	{
		AllowedClasses_SoftObjectPaths.Add(ImplClass.ToSoftObjectPath());
	}

	TArray<FSoftObjectPath> IgnoredClasses_SoftObjectPaths;
	for (const auto& ImplClass : IgnoredClasses)
	{
		IgnoredClasses_SoftObjectPaths.Add(ImplClass.ToSoftObjectPath());
	}

	const auto OnResourceAcquired = [](TArray<TSubclassOf<AActor>>* Resources, TSharedPtr<FStreamableHandle>* Handle)
	{
		if ((Resources == nullptr) || (Handle == nullptr) || !Handle->IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		(*Handle)->GetLoadedAssets(OutResources);

		Resources->Reserve(OutResources.Num());
		for (UObject* Resource : OutResources)
		{
			auto* ImplClass = Cast<UClass>(Resource);
			if (IsValid(ImplClass))
			{
				Resources->Add(ImplClass);
			}
		}
	};

	const auto CallbackA = FStreamableDelegate::CreateWeakLambda(this, OnResourceAcquired, &AllowedActorClasses, &AllowedClasses_StreamableHandle);
	AllowedClasses_StreamableHandle = UAssetManager::Get().LoadAssetList(AllowedClasses_SoftObjectPaths, CallbackA);

	const auto CallbackB = FStreamableDelegate::CreateWeakLambda(this, OnResourceAcquired, &IgnoredActorClasses, &IgnoredClasses_StreamableHandle);
	IgnoredClasses_StreamableHandle = UAssetManager::Get().LoadAssetList(IgnoredClasses_SoftObjectPaths, CallbackB);
}

bool UBatchingRule::DoesQualifyForBatchDestroy(const AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const UClass* InspectClass = Actor->GetClass();
	if (!IsValid(InspectClass))
	{
		return false;
	}

	if (!bAllowBatchDestroyChildClasses)
	{
		const bool bIsIgnored = IgnoredActorClasses.Contains(InspectClass);
		if (bIsIgnored)
		{
			return false;
		}

		return AllowedActorClasses.Contains(InspectClass);
	}
	else
	{
		bool HasChild = false;

		for (const auto& ActorClass : IgnoredActorClasses)
		{
			HasChild |= InspectClass->IsChildOf(ActorClass);
		}

		if (HasChild)
		{
			return false;
		}

		for (const auto& ActorClass : AllowedActorClasses)
		{
			HasChild |= InspectClass->IsChildOf(ActorClass);
		}

		return HasChild;
	}
}

int32 UBatchingRule::GetMaxSizePerBatchDestroy() const
{
	return MaxSizePerBatchDestroy;
}

float UBatchingRule::GetBatchInterval() const
{
	return IntervalBetweenBatchDestroy;
}

float UBatchingRule::GetMaxLifetimeAllowedToUndersizeBatch() const
{
	return MaxLifetimeAllowedToUndersizeBatch;
}

bool UBatchingRule::ShouldGarbageOnNextTick() const
{
	return bShouldGarbageOnNextTick;
}
