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

#include "InventoryProvider.h"
#include "InventorySettings.h"
#include "ItemObject.h"

void UNonReplicatedLoadoutObject::Cycle(const FGameplayTag& TargetTag)
{
	if (!ensureAlwaysMsgf(bDoesSupportItemCycling,
	                      TEXT("Attempt to cycle on an Owning Inventory Component that doesn't support it!")))
	{
		return;
	}

	const bool bDoesContains = CyclingSlots.Contains(TargetTag);
	if (bDoesContains)
	{
		ActiveItemSlotTag = TargetTag;
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

	static const auto ActorSpawnConditions = UInventorySettings::GetItemActorSpawnConditions();
	if (!ensureAlwaysMsgf(!ActorSpawnConditions.IsEmpty(),
	                      TEXT("ActorSpawnConditions missing. Verify your developer settings.")))
	{
		return;
	}

	for (UItemObject* NewItemObject : NewItemObjects)
	{
		if (!IsValid(NewItemObject) || !NewItemObject->DoesRuntimeStateHasPartialMatch(FGameplayTagContainer{UInventorySettings::GetEquippedTag()}))
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

		if (!NewItemObject->DoesBehaviourHasPartialMatch(ActorSpawnConditions))
		{
			continue;
		}

		// @gdemers we need to initialize our loadout with the default element in hand represented
		// by the highest priority tag.
		const bool bShouldDefaultInHand = (bDoesSupportItemCycling && true);
		if (bShouldDefaultInHand)
		{
			Cycle(ItemSlotTag_HighestPriority);
		}
	}
}
