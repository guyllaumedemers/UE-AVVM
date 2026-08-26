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

#include "AVVMDoesActorSupportAnimationInterruption.h"
#include "AVVMLogger.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMToolkitUtils.h"
#include "InventorySampleModule.h"
#include "ItemObject.h"
#include "LoadoutExecutionContextParams.h"
#include "LoadoutExecutionContextRule.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Tags/PrivateTags.h"
#include "UI/LoadoutNotificationPayload.h"

void UNonReplicatedLoadoutObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UNonReplicatedLoadoutObject, ActiveItemSlotTag, Params);
}

bool UNonReplicatedLoadoutObject::IsSupportedForNetworking() const
{
	return true;
}

#if UE_WITH_IRIS
void UNonReplicatedLoadoutObject::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicaitonFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

int32 UNonReplicatedLoadoutObject::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	check(!(Function->FunctionFlags & FUNC_Static));
	check(Function->FunctionFlags & FUNC_Net);

	const AActor* Outer = GetTypedOuter<AActor>();
	const bool bIsOnServer = IsValid(Outer) ? Outer->HasAuthority() : false;

	// get the top most function
	while (Function->GetSuperFunction() != nullptr)
	{
		Function = Function->GetSuperFunction();
	}

	// Multicast RPCs
	if ((Function->FunctionFlags & FUNC_NetMulticast))
	{
		if (bIsOnServer)
		{
			// Server should execute locally and call remotely
			return (FunctionCallspace::Local | FunctionCallspace::Remote);
		}
		else
		{
			return FunctionCallspace::Local;
		}
	}

	// if we are the authority
	if (bIsOnServer)
	{
		if (Function->FunctionFlags & FUNC_NetClient)
		{
			return FunctionCallspace::Remote;
		}
		else
		{
			return FunctionCallspace::Local;
		}

	}
	// if we are not the authority
	else
	{
		if (Function->FunctionFlags & FUNC_NetServer)
		{
			return FunctionCallspace::Remote;
		}
		else
		{
			// don't replicate
			return FunctionCallspace::Local;
		}
	}
}

bool UNonReplicatedLoadoutObject::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack)
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}
	
	const UNetDriver* NetDriver = World->GetNetDriver();
	if (!IsValid(NetDriver))
	{
		return false;
	}
	
	// Retrieves the Iris ReplicationSystem instance tied to this NetDriver
	UReplicationSystem* ReplicationSystem = NetDriver->GetReplicationSystem();
	if (!IsValid(ReplicationSystem))
	{
		return false;
	}
	
	return ReplicationSystem->SendRPC(GetTypedOuter<AActor>(), this, Function, Parms);
}

void UNonReplicatedLoadoutObject::MouseCycle(const float MouseWheelDelta)
{
	auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!CyclingSlots.IsEmpty(),
	                      TEXT("Attempting to cycle on invalid slot collection.")))
	{
		return;
	}

	const int32 Sign = FMath::Sign(MouseWheelDelta);
	const int32 CurrSlotTagIndex = CyclingSlots.IndexOfByKey(ActiveItemSlotTag);
	const int32 NewSlotTagIndex = ((CurrSlotTagIndex + Sign + CyclingSlots.Num()) % CyclingSlots.Num());

	if (!ensureAlwaysMsgf(CyclingSlots.IsValidIndex(NewSlotTagIndex),
	                      TEXT("Invalid Slot Tag.")) || (CurrSlotTagIndex == NewSlotTagIndex))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogInventorySample,
	                Outer,
	                Outer,
	                TEXT("Attempting to modify Active Slot Tag from : %s, to: %s"),
	                *CyclingSlots[CurrSlotTagIndex].ToString(),
	                *CyclingSlots[NewSlotTagIndex].ToString());

	Cycle(CyclingSlots[NewSlotTagIndex]);
}

void UNonReplicatedLoadoutObject::Cycle(const FGameplayTag& TargetTag)
{
	if (!ensureAlwaysMsgf(bDoesSupportItemCycling,
	                      TEXT("Attempting to call Cycle on a instance that isnt supporting it.")))
	{
		return;
	}
	
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
		// @gdemers for server and client, we need to capture the user selection
		// locally, or received via RPC, and execute actions based on the state of the active index, and pending to be active index.
		const int32 NewTargetIndex = CyclingSlots.IndexOfByKey(TargetTag);
		ensureAlwaysMsgf(UAVVMPredictiveInputUtils::Capture(NewTargetIndex, PredictiveInputIndex), TEXT("Invalid Capture operation."));
		
		if (Outer->HasAuthority())
		{
			OnCycle(TargetTag);
		}
		else
		{
			Server_Cycle(TargetTag);
		}
	}
	else
	{
		// @gdemers for server and client, any failure to execute action should result
		// in a complete flush of the predictive queue captured over the last few inputs.
		ensureAlwaysMsgf(UAVVMPredictiveInputUtils::Flush(PredictiveInputIndex), TEXT("Invalid Flush operation."));
	}

	AVVM_LOGGER_LOG(LogInventorySample,
	                Outer,
	                Outer,
	                TEXT("bResult:%d, Active SlotTag is: %s"),
	                static_cast<int32>(bWasSuccess),
	                *ActiveItemSlotTag.ToString());

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
	static const auto ActiveTags = FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_ACTIVE};
	const FGameplayTag OldTag = ActiveItemSlotTag;
	
	MARK_PROPERTY_DIRTY_FROM_NAME(UNonReplicatedLoadoutObject, ActiveItemSlotTag, this);
	ActiveItemSlotTag = TargetTag;

	if (OldTag.IsValid() && ensureAlwaysMsgf(Loadout.Contains(OldTag),
	                                         TEXT("Tag invalid. Loadout doesnt support this slot tag.")))
	{
		auto& OldItem = Loadout[OldTag];
		if (OldItem.IsValid())
		{
			OldItem->ModifyRuntimeState({}, ActiveTags);
		}
	}

	if (TargetTag.IsValid() && ensureAlwaysMsgf(Loadout.Contains(TargetTag),
	                                            TEXT("Tag invalid. Loadout doesnt support this slot tag.")))
	{
		auto& NewItem = Loadout[TargetTag];
		if (NewItem.IsValid())
		{
			NewItem->ModifyRuntimeState(ActiveTags, {});
		}
	}
}

bool UNonReplicatedLoadoutObject::OnIndex_Pause(const int32 TargetIndex)
{
	if (!ActiveItemSlotTag.IsValid())
	{
		return false;
	}

	if (!ensureAlwaysMsgf(Loadout.Contains(ActiveItemSlotTag),
	                      TEXT("Slot Tag missing from loadout Object.")))
	{
		return false;
	}

	auto& ItemObject = Loadout[ActiveItemSlotTag];
	if (!ensureAlwaysMsgf(ItemObject.IsValid(),
	                      TEXT("Invalid ItemObject access.")))
	{
		return false;
	}

	AActor* RuntimeItemActor = ItemObject->GetRuntimeItemActor();

	if (!ensureAlwaysMsgf(IsValid(RuntimeItemActor), TEXT("Invalid ItemActor access.")) ||
		!UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMDoesActorSupportAnimationInterruption>(RuntimeItemActor))
	{
		return false;
	}

	IAVVMDoesActorSupportAnimationInterruption::Execute_Pause(RuntimeItemActor);
	return true;
}

bool UNonReplicatedLoadoutObject::OnIndex_Resume(const int32 TargetIndex)
{
	if (!ensureAlwaysMsgf(CyclingSlots.IsValidIndex(TargetIndex),
	                      TEXT("Invalid Slot Tag Index")))
	{
		return false;
	}

	const FGameplayTag& TargetSlotTag = CyclingSlots[TargetIndex];
	if (!ensureAlwaysMsgf(TargetSlotTag.IsValid(),
	                      TEXT("Invalid Slot Tag.")))
	{
		return false;
	}
	
	if (!ensureAlwaysMsgf(Loadout.Contains(TargetSlotTag),
						  TEXT("Slot Tag missing from loadout Object.")))
	{
		return false;
	}

	auto& ItemObject = Loadout[TargetSlotTag];
	if (!ensureAlwaysMsgf(ItemObject.IsValid(),
						  TEXT("Invalid ItemObject access.")))
	{
		return false;
	}

	AActor* RuntimeItemActor = ItemObject->GetRuntimeItemActor();
	if (!ensureAlwaysMsgf(IsValid(RuntimeItemActor), TEXT("Invalid ItemActor access.")) ||
		!UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMDoesActorSupportAnimationInterruption>(RuntimeItemActor))
	{
		return false;
	}
	
	ActiveItemSlotTag = TargetSlotTag;
	IAVVMDoesActorSupportAnimationInterruption::Execute_Resume(RuntimeItemActor);
	return true;
}

bool UNonReplicatedLoadoutObject::OnIndex_Restart(const int32 TargetIndex)
{
	if (!ensureAlwaysMsgf(CyclingSlots.IsValidIndex(TargetIndex),
	                      TEXT("Invalid Slot Tag Index")))
	{
		return false;
	}

	const FGameplayTag& TargetSlotTag = CyclingSlots[TargetIndex];
	if (!ensureAlwaysMsgf(TargetSlotTag.IsValid(),
	                      TEXT("Invalid Slot Tag.")))
	{
		return false;
	}

	if (!ensureAlwaysMsgf(Loadout.Contains(TargetSlotTag),
	                      TEXT("Slot Tag missing from loadout Object.")))
	{
		return false;
	}

	auto& ItemObject = Loadout[TargetSlotTag];
	if (!ensureAlwaysMsgf(ItemObject.IsValid(),
	                      TEXT("Invalid ItemObject access.")))
	{
		return false;
	}

	// @gdemers imply it is not destroyed between item swap.
	AActor* RuntimeItemActor = ItemObject->GetRuntimeItemActor();
	if (!ensureAlwaysMsgf(IsValid(RuntimeItemActor), TEXT("Invalid ItemActor access.")) ||
		!UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMDoesActorSupportAnimationInterruption>(RuntimeItemActor))
	{
		return false;
	}

	ActiveItemSlotTag = TargetSlotTag;
	IAVVMDoesActorSupportAnimationInterruption::Execute_Restart(RuntimeItemActor);
	return true;
}

bool UNonReplicatedLoadoutObject::OnIndex_Flush(const int32 TargetIndex)
{
	// TODO @gdemers define how flushing should behave
	return true;
}

void UNonReplicatedLoadoutObject::Client_Init()
{
	// @gdemers default init loadout keys
	for (const auto& SlotTag : CyclingSlots)
	{
		Loadout.Add(SlotTag);
	}

	PredictiveInputIndex =
	{
			BIND_PREDICTED_INPUT_INDEX_CHANGED_CLOSURE_TYPE(OnIndex_Pause),
			BIND_PREDICTED_INPUT_INDEX_CHANGED_CLOSURE_TYPE(OnIndex_Resume),
			BIND_PREDICTED_INPUT_INDEX_CHANGED_CLOSURE_TYPE(OnIndex_Restart),
			CyclingSlots.Num()
	};
}

bool UActorLoadoutUtils::DoesActiveItemHasHighestPriority(const TArray<FGameplayTag>& CyclingSlots,
                                                          const FGameplayTag& ActiveItemSlotTag,
                                                          const FGameplayTag& NewItemSlotTag)
{
	const int32 ActiveItemIndex = CyclingSlots.IndexOfByKey(ActiveItemSlotTag);
	const int32 NewItemIndex = CyclingSlots.IndexOfByKey(NewItemSlotTag);
	return !ActiveItemSlotTag.IsValid() || (ActiveItemIndex >= NewItemIndex);
}
