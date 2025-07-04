﻿//Copyright(c) 2025 gdemers
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
#include "ActorInventoryComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "InventoryProvider.h"
#include "ItemObject.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UActorInventoryComponent_InstanceCounter, TEXT("Inventory Component Instance Counter"));

UActorInventoryComponent::UActorInventoryComponent(const FObjectInitializer& ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UActorInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorInventoryComponent, Items);
	DOREPLIFETIME(UActorInventoryComponent, OwnedGameplayTags);
}

void UActorInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UActorInventoryComponent_InstanceCounter);

	LayoutHandler = NewObject<UInventoryLayoutHandler>(this);
	OwningOuter = Outer;

#if WITH_SERVER_CODE
	if (bShouldAsyncLoadOnBeginPlay)
	{
		RequestItems(Outer);
	}
#endif

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorInventoryComponent::StaticClass()->GetName(),
	       *Outer->GetName())
}

void UActorInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto Iterator = Items.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorInventoryComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

void UActorInventoryComponent::RequestItems(const UObject* Outer)
{
	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IInventoryProvider, UInventoryProvider>(Outer);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IInventoryProvider interface!")))
	{
		return;
	}

	const EItemSrcType ItemSrcType = IInventoryProvider::Execute_GetItemSrcType(Outer);
	const bool bIsNone = EnumHasAnyFlags(ItemSrcType, EItemSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IHasItemCollection::GetItemSrcType is None. Check if it was properly overriden.")))
	{
		return;
	}

	const bool bIsItemSrcStatic = EnumHasAnyFlags(ItemSrcType, EItemSrcType::Static);
	if (bIsItemSrcStatic)
	{
		IInventoryProvider::Execute_RequestItemsFromDataAsset(Outer);
	}
	else
	{
		IInventoryProvider::Execute_RequestItemsFromMicroService(Outer);
	}
}

void UActorInventoryComponent::SetupItems(const TArray<UObject*>& Resources)
{
	const AActor* Outer = OwningOuter.Get();
	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<FSoftObjectPath> DeferredItems;
	for (const UObject* Resource : Resources)
	{
		const auto* ItemAsset = Cast<UItemDefinitionDataAsset>(Resource);
		if (!IsValid(ItemAsset))
		{
			continue;
		}

		if (!ItemAsset->CanAccessItem(OwnedGameplayTags, OwnedGameplayTags))
		{
			UE_LOG(LogGameplay,
			       Log,
			       TEXT("Executed from \"%s\". Failed to Meet \"%s\" Requirements."),
			       IsServerOrClientString,
			       *ItemAsset->GetName());

			continue;
		}

		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". New \"%s\" Recorded."),
		       IsServerOrClientString,
		       *ItemAsset->GetName());

		DeferredItems.Add(ItemAsset->GetItemObjectClass().ToSoftObjectPath());
	}

	if (!DeferredItems.IsEmpty())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UActorInventoryComponent::OnItemsRetrieved);
		StreamableHandle = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
	}
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItems() const
{
	return Items;
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItemsByPartialMatch(const FGameplayTagContainer& FilteringTags) const
{
	return Items.FilterByPredicate([Compare = FilteringTags](const UItemObject* Param)
	{
		return IsValid(Param) && Param->HasPartialMatch(Compare);
	});
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItemsByExactMatch(const FGameplayTagContainer& FilteringTags) const
{
	return Items.FilterByPredicate([Compare = FilteringTags](const UItemObject* Param)
	{
		return IsValid(Param) && Param->HasExactMatch(Compare);
	});
}

void UActorInventoryComponent::ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags)
{
	OwnedGameplayTags.RemoveTags(RemovedTags);
	OwnedGameplayTags.AppendTags(AddedTags);
}

bool UActorInventoryComponent::HasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return OwnedGameplayTags.HasAnyExact(Compare);
}

bool UActorInventoryComponent::HasExactMatch(const FGameplayTagContainer& Compare) const
{
	return OwnedGameplayTags.HasAllExact(Compare);
}

void UActorInventoryComponent::OnItemsRetrieved()
{
	if (!ensureAlwaysMsgf(StreamableHandle.IsValid(), TEXT("UActorInventoryComponent::OnItemsRetrieved Streamable handle is invalid!")))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	// @gdemers remove first
	for (auto Iterator = Items.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	TArray<UObject*> OutStreamableAssets;
	StreamableHandle->GetLoadedAssets(OutStreamableAssets);

	Items.Reset(OutStreamableAssets.Num());
	// @gdemers append after
	for (UObject* StreamableAsset : OutStreamableAssets)
	{
		auto* Item = Cast<UItemObject>(StreamableAsset);
		if (IsValid(Item))
		{
			AddReplicatedSubObject(Item);
			Items.Add(Item);
		}
	}

	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IInventoryProvider, UInventoryProvider>(Outer);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IInventoryProvider interface!")))
	{
		return;
	}

	const bool bDoesSupportSpawnOnLoad = IInventoryProvider::Execute_DoesSupportSpawnOnLoad(Outer);
	if (!bDoesSupportSpawnOnLoad)
	{
		return;
	}

	// @gdemers handle spawning default object that are currently equipped
	for (UItemObject* Item : Items)
	{
		const bool bIsItemEquipped = IInventoryProvider::Execute_IsItemEquipped(Outer, Item);
		if (bIsItemEquipped)
		{
			Item->TrySpawnEquippedItem(Outer);
		}
	}
}

void UActorInventoryComponent::OnRep_ItemCollectionChanged(const TArray<UItemObject*>& OldItemObjects)
{
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	const int32 OldSize = OldItemObjects.Num();
	const int32 NewSize = Items.Num();

	FStringView SV;
	if (NewSize == OldSize)
	{
		SV = TEXT("has identical Size. We may have Swapped Items!");
	}
	else if (NewSize > OldSize)
	{
		SV = TEXT("has increased!");
	}
	else
	{
		SV = TEXT("has decreased!");
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Item Collection modified on Outer \"%s\"! Collection %s"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName(),
	       SV.GetData());
}
