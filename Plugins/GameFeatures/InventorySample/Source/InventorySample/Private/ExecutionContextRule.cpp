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
#include "ExecutionContextRule.h"

#include "ActorInventoryComponent.h"
#include "ExecutionContextParams.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "NativeGameplayTags.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ACTION_DROP_BLOCKED, "InventorySample.Item.Drop.Blocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ACTION_PICKUP_BLOCKED, "InventorySample.Item.Pickup.Blocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ACTION_PICKUP_INTO_SLOT_SWAP, "InventorySample.Item.Pickup.IntoSlotSwap");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ACTION_PICKUP_INTO_BACKBACK_STORAGE, "InventorySample.Item.Pickup.IntoBackpackStorage");

UScriptStruct* TBaseStructure<FExecutionContextRule>::Get()
{
	return FExecutionContextRule::StaticStruct();
}

bool FDropRule::Predicate(const UActorInventoryComponent* InventoryComponent,
                          const TInstancedStruct<FExecutionContextParams>& Params) const
{
	if (!IsValid(InventoryComponent))
	{
		return false;
	}

	const auto* DropParams = Params.GetPtr<FDropContextParams>();
	if (!ensureAlwaysMsgf(DropParams != nullptr,
	                      TEXT("FExecutionContextParams couldn't be cast to FDropContextParams.")))
	{
		return false;
	}

	const UItemObject* PendingDropItemObject = DropParams->ItemObject.Get();
	if (!IsValid(PendingDropItemObject))
	{
		return false;
	}

	// @gdemers IMPORTANT : RuntimeCount is handled within the UItemObject, and will apply the expected
	// tags to prevent actions from being executed on a zero count element.
	const FGameplayTagContainer BlockedTags = FGameplayTagContainer(TAG_INVENTORY_ACTION_DROP_BLOCKED);
	if (PendingDropItemObject->DoesBehaviourHasPartialMatch(BlockedTags) /*blocked by design*/ ||
		PendingDropItemObject->DoesRuntimeStateHasPartialMatch(BlockedTags)/*blocked by another action*/)
	{
		return false;
	}

	// @gdemers more advanced dropping mechanism involving the owning outer should be handled within the
	// FDropRule context derived type that match design requirements.
	return true;
}

bool FPickupRule::Predicate(const UActorInventoryComponent* InventoryComponent,
                            const TInstancedStruct<FExecutionContextParams>& Params) const
{
	if (!IsValid(InventoryComponent))
	{
		return false;
	}

	const auto* PickupParams = Params.GetPtr<FPickupContextParams>();
	if (!ensureAlwaysMsgf(PickupParams != nullptr,
	                      TEXT("FExecutionContextParams couldn't be cast to FPickupContextParams.")))
	{
		return false;
	}

	const UItemObject* PendingPickupItemObject = PickupParams->ItemObject.Get();
	if (!IsValid(PendingPickupItemObject))
	{
		return false;
	}

	const FGameplayTagContainer BlockedTags = FGameplayTagContainer(TAG_INVENTORY_ACTION_PICKUP_BLOCKED);
	if (PendingPickupItemObject->DoesRuntimeStateHasPartialMatch(BlockedTags)/*maybe it cannot be interacted with yet*/)
	{
		return false;
	}

	// @gdemers world items can be either swap with currently equipped element, or put into storage held by outer.
	FGameplayTagContainer PickupActions = FGameplayTagContainer::EmptyContainer;
	PickupActions.AddTag(TAG_INVENTORY_ACTION_PICKUP_INTO_BACKBACK_STORAGE);
	PickupActions.AddTag(TAG_INVENTORY_ACTION_PICKUP_INTO_SLOT_SWAP);

	if (!PendingPickupItemObject->DoesBehaviourHasPartialMatch(PickupActions))
	{
		return false;
	}

	// @gdemers validate the inventory state. Are we already full ?
	if (InventoryComponent->HasPartialMatch(UInventorySettings::GetFullInventory_BlockedActions()) ||
		InventoryComponent->CheckWeightOverflow(PendingPickupItemObject)/*validate possible weight overflow*/)
	{
		return false;
	}

	return true;
}

bool FSwapRule::Predicate(const UActorInventoryComponent* InventoryComponent,
                          const TInstancedStruct<FExecutionContextParams>& Params) const
{
	if (!IsValid(InventoryComponent))
	{
		return false;
	}

	return true;
}
