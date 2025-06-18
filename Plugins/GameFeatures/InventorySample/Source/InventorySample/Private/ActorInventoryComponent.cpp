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
#include "ActorInventoryComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "InventoryProvider.h"
#include "ItemObject.h"
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
		FOnRetrieveInventoryItems Callback;
		Callback.BindDynamic(this, &UActorInventoryComponent::OnItemsRetrieved);
		UInventoryBlueprintFunctionLibrary::RequestItems(Outer, Callback);
	}
#endif

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UActorInventoryComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())
}

void UActorInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_SERVER_CODE
	for (auto Iterator = Items.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}
#endif

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UActorInventoryComponent from Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItems() const
{
	return Items;
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItemsByPartialMatch(const FGameplayTagContainer& FilteringTags) const
{
	return Items.FilterByPredicate([Compare = FilteringTags](const UItemObject* Item)
	{
		return IsValid(Item) && Item->HasPartialMatch(Compare);
	});
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItemsByExactMatch(const FGameplayTagContainer& FilteringTags) const
{
	return Items.FilterByPredicate([Compare = FilteringTags](const UItemObject* Item)
	{
		return IsValid(Item) && Item->HasExactMatch(Compare);
	});
}

void UActorInventoryComponent::OnItemsRetrieved(const TArray<UItemObject*>& ItemObjects)
{
	if (!ensureAlwaysMsgf(!ItemObjects.IsEmpty(), TEXT("UActorInventoryComponent::OnItemsRetrieved has received an Empty Collection!")))
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

	Items.Reset(ItemObjects.Num());
	// @gdemers append after
	for (UItemObject* Item : ItemObjects)
	{
		if (IsValid(Item) /*Make Array in BP return size=1 array with [0]=nullptr*/)
		{
			AddReplicatedSubObject(Item);
			Items.Add(Item);
		}
	}

	// @gdemers handle spawning default object that are currently equipped
	for (UItemObject* Item : Items)
	{
		if (IsValid(Item))
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
	       TEXT("Executed from \"%s\". Item Collection modified on Actor \"%s\"! Collection %s"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName(),
	       SV.GetData());
}
