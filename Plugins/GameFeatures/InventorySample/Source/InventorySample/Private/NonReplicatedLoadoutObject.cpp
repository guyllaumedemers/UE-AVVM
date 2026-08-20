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

#include "AVVMNotificationSubsystem.h"
#include "ItemObject.h"
#include "LoadoutExecutionContextParams.h"
#include "LoadoutExecutionContextRule.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Tags/PrivateTags.h"
#include "UI/LoadoutNotificationPayload.h"

void UNonReplicatedLoadoutObject::Cycle(const FGameplayTag& TargetTag)
{
	const AActor* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer),
	                      TEXT("Invalid Outer!")))
	{
		return;
	}

	const auto Ctx = FAVVMExecutionContextParams::Make<FLoadoutExecutionContextParams>(ActiveItemSlotTag, TargetTag);
	const auto Rule = GetEquipRule();
	const bool bWasSuccess = UAVVMExecutionContextUtils::CanExecute(this, Ctx, Rule);
	if (bWasSuccess)
	{
		if (Outer->HasAuthority())
		{
			OnCycle(TargetTag);
		}
		else
		{
			// TODO @gdemers We need to support predictive exchange of loadout items so the equip/unequip
			// animation sequence can smoothly be transition on client, and run latent validation with server to reconciliate
			// the end pose in which the play is expected to be on Server.
			// i.e For when local client equip/unequip rapidly for showing off (Ping-Pong effect).
			Server_Cycle(TargetTag);
		}
	}

#if WITH_EDITOR
	if (!Outer->IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_INVENTORYSAMPLE_ITEM_NOTIFICATION_SWAP,
		                                        Outer->GetTypedOuter<APlayerController>(),
		                                        Outer,
		                                        FAVVMNotificationPayload::Make<FLoadoutNotificationPayload>(ActiveItemSlotTag, TargetTag, bWasSuccess));
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
	FGameplayTagContainer Requirements;
	Requirements.AddTag(TAG_INVENTORYSAMPLE_ITEM_STATE_PENDING_SPAWN);
	Requirements.AddTag(TAG_INVENTORYSAMPLE_ITEM_STATE_INSTANCED);

	for (const UItemObject* OldItemObject : OldItemObjects)
	{
		if (!IsValid(OldItemObject) || NewItemObjects.Contains(OldItemObject)/*persist between collection change*/)
		{
			continue;
		}

		// @gdemers we only care about items that are Instanced. i.e with physical representation in world, and slotted with our loadout system.
		// other items in our inventory are not participating in this system behaviour.
		// Note : Holstered items are still instanced. to prevent visual problem, item spawned in world that support quick swap
		// should only be hidden/shown based on their active state.
		if (!OldItemObject->DoesRuntimeStateHasPartialMatch(Requirements))
		{
			continue;
		}

		const FGameplayTag& ActiveSlotTag = OldItemObject->GetRuntimeItemSlotTag();
		if (!ActiveSlotTag.IsValid()/*we may not be attached to a slot, and is being dropped*/)
		{
			continue;
		}

		const bool bHasKey = Loadout.Contains(ActiveSlotTag);
		if (!ensureAlwaysMsgf(bHasKey, TEXT("Invalid Key search. Make sure we have a valid RuntimeSlotTag.")))
		{
			continue;
		}

		const bool bAreEqual = (Loadout[ActiveSlotTag] == OldItemObject);
		if (bAreEqual)
		{
			Loadout[ActiveSlotTag].Reset();
		}
	}
}

void UNonReplicatedLoadoutObject::ModifyLoadout(const TArray<UItemObject*>& NewItemObjects)
{
	FGameplayTagContainer Requirements;
	Requirements.AddTag(TAG_INVENTORYSAMPLE_ITEM_STATE_PENDING_SPAWN);
	Requirements.AddTag(TAG_INVENTORYSAMPLE_ITEM_STATE_INSTANCED);

	for (UItemObject* NewItemObject : NewItemObjects)
	{
		// @gdemers we only care about items that are Instanced. i.e with physical representation in world, and slotted with our loadout system.
		// other items in our inventory are not participating in this system behaviour.
		// Note : Holstered items are still instanced. to prevent visual problem, item spawned in world that support quick swap
		// should only be hidden/shown based on their active state.
		if (!IsValid(NewItemObject) || !NewItemObject->DoesRuntimeStateHasPartialMatch(Requirements))
		{
			continue;
		}

		// @gdemers a new item being registered with this system should ALWAYS have a valid slot tag.
		const FGameplayTag& TargetSlotTag = NewItemObject->GetRuntimeItemSlotTag();
		if (!ensureAlwaysMsgf(TargetSlotTag.IsValid(), TEXT("Active Item is missing a valid slot tag.")))
		{
			continue;
		}

		auto& OutValue = Loadout.FindOrAdd(TargetSlotTag);
		OutValue = NewItemObject;

		if (!bDoesSupportItemCycling || !NewItemObject->DoesBehaviourHasPartialMatch(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_ACTIVE}))
		{
			continue;
		}

		// @gdemers we need to initialize our loadout with the default element in hand represented by the highest priority tag.
		const bool bDoesActiveItemHasHighestPriority = UActorLoadoutUtils::DoesActiveItemHasHighestPriority(CyclingSlots, ActiveItemSlotTag, TargetSlotTag);
		if (!bDoesActiveItemHasHighestPriority)
		{
			Cycle(TargetSlotTag);
		}
	}
}

void UNonReplicatedLoadoutObject::Server_Cycle_Implementation(const FGameplayTag& TargetTag)
{
	Cycle(TargetTag);
}

TInstancedStruct<FAVVMExecutionContextRule> UNonReplicatedLoadoutObject::GetUnequipRule() const
{
	return FAVVMExecutionContextRule::Make<FLoadoutUnequipRule>();
}

TInstancedStruct<FAVVMExecutionContextRule> UNonReplicatedLoadoutObject::GetEquipRule() const
{
	return FAVVMExecutionContextRule::Make<FLoadoutEquipRule>();
}

void UNonReplicatedLoadoutObject::OnCycle(const FGameplayTag& TargetTag)
{
	if (!ensureAlwaysMsgf(bDoesSupportItemCycling,
	                      TEXT("Attempt to cycle on an Owning Inventory Component that doesn't support it!")))
	{
		return;
	}

	static const auto ActiveTags = FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_ACTIVE};
	const FGameplayTag& OldTag = ActiveItemSlotTag;
	const FGameplayTag& NewTag = ActiveItemSlotTag = TargetTag;

	// TODO @gdemers We need to be able to queue incoming exchange (equip/unequip), and cancel running animation
	// during animation cancellation due to ping-pong effect. 
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

bool UActorLoadoutUtils::DoesActiveItemHasHighestPriority(const TArray<FGameplayTag>& CyclingSlots,
                                                          const FGameplayTag& ActiveItemSlotTag,
                                                          const FGameplayTag& NewItemSlotTag)
{
	const int32 ActiveItemIndex = CyclingSlots.IndexOfByKey(ActiveItemSlotTag);
	const int32 NewItemIndex = CyclingSlots.IndexOfByKey(NewItemSlotTag);
	return (ActiveItemIndex >= NewItemIndex);
}
