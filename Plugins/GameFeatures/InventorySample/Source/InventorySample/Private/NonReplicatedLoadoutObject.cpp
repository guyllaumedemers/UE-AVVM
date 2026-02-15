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
#include "NonReplicatedLoadoutObject.h"

#include "InventorySettings.h"
#include "ItemObject.h"
#include "Tags/PrivateTags.h"

void UNonReplicatedLoadoutObject::Cycle(const FGameplayTag& TargetTag)
{
	if (!ensureAlwaysMsgf(bDoesSupportItemCycling,
	                      TEXT("Attempt to cycle on an Owning Inventory Component that doesn't support it!")))
	{
		return;
	}

	static const auto ActiveTags = FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_ACTIVE};
	const FGameplayTag& OldTag = ActiveItemSlotTag;
	const FGameplayTag& NewTag = ActiveItemSlotTag = TargetTag;

	if (CyclingSlots.Contains(OldTag))
	{
		auto& OldItem = Loadout[OldTag];
		if (OldItem.IsValid())
		{
			OldItem->ModifyRuntimeState({}, ActiveTags);
		}
	}

	if (CyclingSlots.Contains(NewTag))
	{
		auto& NewItem = Loadout[NewTag];
		if (NewItem.IsValid())
		{
			NewItem->ModifyRuntimeState(ActiveTags, {});
		}
	}
}

void UNonReplicatedLoadoutObject::HandleItemCollectionChanged(const TArray<UItemObject*>& NewItemObjects,
                                                              const TArray<UItemObject*>& OldItemObjects)
{
	RemoveOldItems(NewItemObjects, OldItemObjects);
	ModifyLoadout(NewItemObjects);
}

void UNonReplicatedLoadoutObject::RemoveOldItems(const TArray<UItemObject*>& NewItemObjects,
                                                 const TArray<UItemObject*>& OldItemObjects)
{
	for (const UItemObject* OldItemObject : OldItemObjects)
	{
		if (!IsValid(OldItemObject))
		{
			continue;
		}

		const bool bDoesContains = NewItemObjects.Contains(OldItemObject);
		if (bDoesContains)
		{
			continue;
		}

		const FGameplayTagContainer& ItemSlotTags = OldItemObject->GetItemSlotTags();
		if (!ensureAlwaysMsgf(!ItemSlotTags.IsEmpty(),
		                      TEXT("ItemSlotTag missing. Validate your Item configuration.")))
		{
			continue;
		}
		
		// @gdemers its important to enforce that the tag order be from highest priority to lowest.
		// example : PrimarySlot, SecondarySlot, etc...
		// an Item can be placed in both Primary, and secondary so to allow context switch between both entry
		// but thats only relevant for the UI, not for gameplay. game play only care about setting the active slot
		// to the user selection, they dont need to modify the object reference attached to the slot tag.
		const FGameplayTag ItemSlotTag_HighestPriority = ItemSlotTags.First();
		const bool bHasKey = Loadout.Contains(ItemSlotTag_HighestPriority);
		if (!bHasKey)
		{
			continue;
		}

		const bool bAreEqual = (Loadout[ItemSlotTag_HighestPriority] == OldItemObject);
		if (bAreEqual)
		{
			Loadout[ItemSlotTag_HighestPriority].Reset();
		}
	}
}

void UNonReplicatedLoadoutObject::ModifyLoadout(const TArray<UItemObject*>& NewItemObjects)
{
	const AActor* Outer = GetTypedOuter<AActor>();
	if (!IsValid(Outer))
	{
		return;
	}

	for (UItemObject* NewItemObject : NewItemObjects)
	{
		if (!IsValid(NewItemObject) || !NewItemObject->DoesRuntimeStateHasPartialMatch(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_EQUIPPED}))
		{
			return;
		}

		const FGameplayTagContainer& ItemSlotTags = NewItemObject->GetItemSlotTags();
		if (!ensureAlwaysMsgf(!ItemSlotTags.IsEmpty(),
		                      TEXT("ItemSlotTag missing. Validate your Item configuration.")))
		{
			continue;
		}

		// @gdemers its important to enforce that the tag order be from highest priority to lowest.
		// example : PrimarySlot, SecondarySlot, etc...
		// an Item can be placed in both Primary, and secondary so to allow context switch between both entry
		// but thats only relevant for the UI, not for gameplay. game play only care about setting the active slot
		// to the user selection, they dont need to modify the object reference attached to the slot tag.
		const FGameplayTag ItemSlotTag_HighestPriority = ItemSlotTags.First();
		auto& OutValue = Loadout.FindOrAdd(ItemSlotTag_HighestPriority);
		OutValue = NewItemObject;

		if (!bDoesSupportItemCycling || !NewItemObject->DoesBehaviourHasPartialMatch(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_BEHAVIOUR_CAN_SPAWN_ACTOR}))
		{
			continue;
		}

		// @gdemers we need to initialize our loadout with the default element in hand represented by the highest priority tag.
		const bool bDoesActiveItemHasHighestPriority = UActorLoadoutUtils::DoesActiveItemHasHighestPriority(CyclingSlots, ActiveItemSlotTag, ItemSlotTag_HighestPriority);
		if (!bDoesActiveItemHasHighestPriority)
		{
			Cycle(ItemSlotTag_HighestPriority);
		}
	}
}

bool UActorLoadoutUtils::DoesActiveItemHasHighestPriority(const TArray<FGameplayTag>& CyclingSlots,
                                                          const FGameplayTag& ActiveItemSlotTag,
                                                          const FGameplayTag& NewItemSlotTag)
{
	const int32 ActiveItemIndex = CyclingSlots.IndexOfByKey(ActiveItemSlotTag);
	const int32 NewItemIndex = CyclingSlots.IndexOfByKey(NewItemSlotTag);
	return (ActiveItemIndex >= NewItemIndex);
}
