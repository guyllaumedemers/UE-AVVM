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
#include "ItemObject.h"

#include "ActorInventoryComponent.h"
#include "AVVMGameplayUtils.h"
#include "AVVMSocketTargetingHelper.h"
#include "InventoryManagerSubsystem.h"
#include "InventorySample.h"
#include "InventorySettings.h"
#include "InventoryUtils.h"
#include "NativeGameplayTags.h"
#include "PickupActor.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/ItemStackTableRow.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Net/UnrealNetwork.h"
#include "Resources/AVVMResourceProvider.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ITEM_DESTROY_CONDITION_ON_EMPTY_STACK, "InventorySample.Item.DestroyConditions.EmptyStack");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ITEM_STACKABLE, "InventorySample.Item.Stackable");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ITEM_STORAGE, "InventorySample.Item.Storage");

void UItemObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UItemObject, RuntimeItemActor, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UItemObject, RuntimeItemState, Params);
}

bool UItemObject::IsSupportedForNetworking() const
{
	return true;
}

#if UE_WITH_IRIS
void UItemObject::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicaitonFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UItemObject::ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags)
{
	RuntimeItemState.StateTags.RemoveTags(RemovedTags);
	RuntimeItemState.StateTags.AppendTags(AddedTags);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(RuntimeItemState);
}

void UItemObject::ModifyRuntimeStorageId(const int32 NewStorageId)
{
	static constexpr int32 MaxStorageId = (1 << GET_STORAGE_ID_ENCODING_BIT_RANGE);
	RuntimeItemState.StorageId = FMath::Clamp<int32>(NewStorageId, 0, MaxStorageId);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(RuntimeItemState);
}

void UItemObject::ModifyRuntimeStoragePosition(const int32 NewStoragePosition)
{
	const int32 MaxStorageCapacity = GetStorageMaxCapacity();
	RuntimeItemState.StoragePosition = FMath::Clamp<int32>(NewStoragePosition, 0, MaxStorageCapacity);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(RuntimeItemState);
}

void UItemObject::ModifyRuntimeStackCount(const int32 NewStackCount)
{
	const int32 MaxStackCount = GetMaxStackCount();
	RuntimeItemState.StackCount = FMath::Clamp<int32>(NewStackCount, 0, MaxStackCount);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(RuntimeItemState);

	// @gdemers early out on non-empty stack.
	if (!!RuntimeItemState.StackCount)
	{
		return;
	}

	// @gdemers check if we are meeting design conditions for destroying this item from inventory.
	static const auto DestroyConditions = FGameplayTagContainer(TAG_INVENTORY_ITEM_DESTROY_CONDITION_ON_EMPTY_STACK);
	const bool bShouldDestroyOnEmptyStack = DoesBehaviourHasPartialMatch(DestroyConditions);

	// @gdemers invalidate any actions we could be executing while pending to destroy, or while the stack is empty.
	const FGameplayTagContainer& BlockingTags = UInventorySettings::GetEmptyItemCount_BlockedActions();
	ModifyRuntimeState(BlockingTags, {});

	if (bShouldDestroyOnEmptyStack)
	{
		const UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			return;
		}

		const auto OnDestroyConditionMet = [](const TWeakObjectPtr<UItemObject>& PendingDestroyItemObject)
		{
			UItemObjectUtils::RuntimeDestroy(PendingDestroyItemObject.Get());
		};

		const auto Callback = FTimerDelegate::CreateWeakLambda(this, OnDestroyConditionMet, TWeakObjectPtr(this));
		World->GetTimerManager().SetTimerForNextTick(Callback);
	}
}

bool UItemObject::DoesRuntimeStateEquals(const FGameplayTagContainer& Compare) const
{
	return (Compare.Num() == RuntimeItemState.StateTags.Num()) && Compare.HasAllExact(RuntimeItemState.StateTags);
}

bool UItemObject::DoesTypeHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(ItemTypeTags);
}

bool UItemObject::DoesTypeHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(ItemTypeTags);
}

bool UItemObject::DoesBehaviourHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(ItemBehaviourTypeTags);
}

bool UItemObject::DoesBehaviourHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(ItemBehaviourTypeTags);
}

bool UItemObject::DoesAttachmentSlotHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(ItemAttachmentSlotTags);
}

bool UItemObject::DoesAttachmentSlotHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(ItemAttachmentSlotTags);
}

bool UItemObject::DoesSlotHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(ItemSlotTags);
}

bool UItemObject::DoesSlotHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(ItemSlotTags);
}

const FGameplayTagContainer& UItemObject::GetRuntimeState() const
{
	return RuntimeItemState.StateTags;
}

bool UItemObject::DoesRuntimeStateHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(RuntimeItemState.StateTags);
}

bool UItemObject::DoesRuntimeStateHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(RuntimeItemState.StateTags);
}

bool UItemObject::IsEmpty() const
{
	return (false == !!RuntimeItemState.StackCount);
}

bool UItemObject::CanStack(const UItemObject* Item) const
{
	// @gdemers validate if both items are of same types.
	if (!IsValid(Item) || (Item->GetItemActorId() != GetItemActorId()))
	{
		return false;
	}

	static const auto StackableTagContainer = FGameplayTagContainer(TAG_INVENTORY_ITEM_STACKABLE);
	const bool bDoesItemStack = DoesBehaviourHasPartialMatch(StackableTagContainer);
	if (!bDoesItemStack)
	{
		return false;
	}

	const int32 MaxStackCount = GetMaxStackCount();
	const int32 TotalStackCount = (Item->GetRuntimeCount() + GetRuntimeCount());

	// @gdemers validate if the total count is lesser than the MaxStackCount.
	if (TotalStackCount > MaxStackCount)
	{
		return false;
	}

	return true;
}

bool UItemObject::Stack(UItemObject* Item)
{
	if (!IsValid(Item))
	{
		return false;
	}

	// @gdemers get stack count from sum between both objects.
	const int32 TotalStackCount = (Item->GetRuntimeCount() + GetRuntimeCount());
	const int32 ClampedStackCount = FMath::Clamp(TotalStackCount, 0, GetMaxStackCount());

	// @gdemers update stack internal representation. Note : the function input may overflow the stack
	// 'this' UItemObject reference.
	Item->ModifyRuntimeStackCount((TotalStackCount - ClampedStackCount)/*handle left-over*/);
	ModifyRuntimeStackCount(ClampedStackCount);

	const bool bDoesStackOverflow = (TotalStackCount > ClampedStackCount);
	return bDoesStackOverflow;
}

int32 UItemObject::GetRuntimeCount() const
{
	return RuntimeItemState.StackCount;
}

int32 UItemObject::GetStorageId() const
{
	return RuntimeItemState.StorageId;
}

int32 UItemObject::GetStoragePosition() const
{
	return RuntimeItemState.StoragePosition;
}

const FDataRegistryId& UItemObject::GetItemActorId() const
{
	return ItemActorId;
}

void UItemObject::GetItemActorClassAsync(const UObject* NewActorDefinitionDataAsset,
                                         const FOnRequestItemActorClassComplete& Callback)
{
	const auto* ActorDefinitionDataAsset = Cast<UAVVMActorDefinitionDataAsset>(NewActorDefinitionDataAsset);
	if (!IsValid(ActorDefinitionDataAsset))
	{
		return;
	}

	ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()}, {});

	FStreamableDelegate OnRequestItemActorClassComplete;
	OnRequestItemActorClassComplete.BindUObject(this, &UItemObject::OnItemActorClassAcquired, Callback, ActorDefinitionDataAsset->GetActorAttributeSetClassSoftObjectPath());
	ItemActorHandle = UAssetManager::Get().LoadAssetList({ActorDefinitionDataAsset->GetActorClassSoftObjectPath()}, OnRequestItemActorClassComplete);
}

void UItemObject::SpawnActor(const FItemActorSpawnContextArgs& ContextArgs)
{
	const UClass* ActorClass = ContextArgs.ActorClass;
	AActor* Outer = ContextArgs.Outer.Get();

	if (!IsValid(ActorClass) || !IsValid(Outer))
	{
		return;
	}

	RuntimeItemActor = UInventoryManagerSubsystem::Static_CreateItemActor(GetWorld(), ActorClass, Outer);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemActor, this);

	if (!ensureAlwaysMsgf(IsValid(RuntimeItemActor),
	                      TEXT("Item Actor Class Failed to create an instance in World!")))
	{
		return;
	}

	const bool bShouldSpawnAndAttach = RuntimeItemState.StateTags.HasAllExact(FGameplayTagContainer{UInventorySettings::GetEquippedTag()});
	ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetInstancedTag()}, FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()});

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Executed from \"%s\". Adding New Visual Actor \"%s\" on Item \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *RuntimeItemActor->GetName(),
	       *GetName());

	const bool bShouldNotifyWhenRegisteringAttachment = RuntimeItemActor->Implements<UAVVMDoesSupportAttachmentNotify>();
	if (bShouldNotifyWhenRegisteringAttachment)
	{
		const auto OnSocketAttached = IAVVMDoesSupportAttachmentNotify::FOnNewSocketAttachedDelegate::FDelegate::CreateUObject(this, &UItemObject::OnNewSocketItemAttached);
		const auto OnSocketDetached = IAVVMDoesSupportAttachmentNotify::FOnNewSocketDetachedDelegate::FDelegate::CreateUObject(this, &UItemObject::OnNewSocketItemDetached);

		auto Observer = TScriptInterface<IAVVMDoesSupportAttachmentNotify>(RuntimeItemActor);
		OnNewSocketAttachedHandle = Observer->OnNewSocketAttachedDelegate_Add(OnSocketAttached);
		OnNewSocketDetachedHandle = Observer->OnNewSocketDetachedDelegate_Add(OnSocketDetached);
	}

	bool bCanRegisterAttributeSet = true;
	if (bShouldSpawnAndAttach)
	{
		FAVVMSocketTargetingDeferralContextArgs Params;
		Params.Parent = Outer;
		Params.SocketName = SocketName;
		// TODO @gdemers : will require proper tag handling later. keep gameplay tag container for now until requirements
		// changes.
		Params.AttachmentSlotTag = ItemAttachmentSlotTags.First();
		Params.SrcAttributeSetSoftObjectPath = ContextArgs.AttributeSetSoftObjectPath;

		// @gdemers We use this so we can handle more complex case that require traversal of our root actor
		// to find attached actors, and used them as targets.
		bCanRegisterAttributeSet = FAVVMSocketTargetingHelper::Static_AttachToActorAsync(RuntimeItemActor, Params);
	}

	const FSoftObjectPath& AttributeSetSoftObjectPath = ContextArgs.AttributeSetSoftObjectPath;
	// @gdemers IMPORTANT : Keep in mind that deferred actor being attached, may or may not own an ASC. As such,
	// handling the initialization of the Attribute set for the delayed case has to be accounted for in the implementer
	// of the IAVVMDoesSupportInnerSocketTargeting interface. This is why we are forwarding above. Below is the case for
	// an actor that has a valid socket at creation time.
	if (!bCanRegisterAttributeSet || AttributeSetSoftObjectPath.IsNull())
	{
		return;
	}

	UAVVMAbilitySystemComponent* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(RuntimeItemActor);
	if (IsValid(ASC))
	{
		ASC->SetupAttributeSet(ContextArgs.AttributeSetSoftObjectPath, RuntimeItemActor);
	}
}

const TMap<FGameplayTag, TWeakObjectPtr<const AActor>>& UItemObject::GetNonReplicatedItemAttachmentActors() const
{
	return NonReplicatedItemAttachmentActors;
}

void UItemObject::OnItemActorClassAcquired(FOnRequestItemActorClassComplete Callback, const FSoftObjectPath NewActorAttributeSetSoftObjectPath)
{
	if (!ItemActorHandle.IsValid())
	{
		Callback.ExecuteIfBound(nullptr, FSoftObjectPath(), this);
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	ItemActorHandle->GetLoadedAssets(OutStreamableAssets);

	if (!OutStreamableAssets.IsEmpty())
	{
		Callback.ExecuteIfBound(Cast<UClass>(OutStreamableAssets[0]), NewActorAttributeSetSoftObjectPath, this);
	}
	else
	{
		Callback.ExecuteIfBound(nullptr, FSoftObjectPath(), this);
	}
}

void UItemObject::OnRep_ItemStateModified(const FItemState& OldItemState)
{
	const bool bHasModifiedState = !DoesRuntimeStateEquals(OldItemState.StateTags);
	if (bHasModifiedState)
	{
		OnItemRuntimeStateChanged.Broadcast(RuntimeItemState.StateTags);
	}

	const bool bHasDifferentCount = (RuntimeItemState.StackCount != OldItemState.StackCount);
	if (bHasDifferentCount)
	{
		OnItemRuntimeCountChanged.Broadcast(RuntimeItemState.StackCount);
	}
}

void UItemObject::OnNewSocketItemAttached(const FGameplayTag& NewItemAttachmentSlotTag,
                                          const AActor* NewAttachment)
{
	NonReplicatedItemAttachmentActors.FindOrAdd(NewItemAttachmentSlotTag, NewAttachment);
}

void UItemObject::OnNewSocketItemDetached(const FGameplayTag& NewItemAttachmentSlotTag)
{
	NonReplicatedItemAttachmentActors.Remove(NewItemAttachmentSlotTag);
}

int32 UItemObject::GetStorageMaxCapacity() const
{
	const int32 StorageMaxCapacity = UItemObjectUtils::GetStorageMaxCapacity(GetTypedOuter<UActorInventoryComponent>(), RuntimeItemState.StorageId);
	if (!ensureAlwaysMsgf((StorageMaxCapacity != INDEX_NONE),
	                      TEXT("StorageId provided couldn't resolve a valid Storage Capacity.")))
	{
		return INDEX_NONE;
	}

	return StorageMaxCapacity;
}

int32 UItemObject::GetMaxStackCount() const
{
	static const auto StackableTagContainer = FGameplayTagContainer(TAG_INVENTORY_ITEM_STACKABLE);
	const bool bDoesStack = DoesBehaviourHasPartialMatch(StackableTagContainer);
	if (!bDoesStack)
	{
		static constexpr int32 One = 1;
		return One;
	}
	else
	{
		const bool bIsStorage = DoesTypeHasPartialMatch(FGameplayTagContainer(TAG_INVENTORY_ITEM_STORAGE));
		static constexpr int32 MaxStorageCapacityBounds = (1 << GET_ITEM_POSITION_ENCODING_BIT_RANGE);
		static constexpr int32 MaxStackCountBounds = (1 << GET_ITEM_COUNT_ENCODING_BIT_RANGE);

		// @gdemers shouldnt require async loading as this table will ever only contains integers.
		const TSoftObjectPtr<UDataTable>& DataTable = UInventorySettings::GetItemMaxStackCountDataTable();
		const UDataTable* MaxCountDataTable = DataTable.LoadSynchronous();

		const int32 MaxStackCount = UItemObjectUtils::GetMaxStackCount(MaxCountDataTable, MaxStackCount_CategoryTag);
		const int32 BitEncodingLimit = (bIsStorage ? MaxStorageCapacityBounds : MaxStackCountBounds);

		return FMath::Clamp(MaxStackCount, 0, BitEncodingLimit);
	}
}

int32 UItemObjectUtils::RuntimeInit(const UObject* Outer,
                                    const TArray<int32>& NewPrivateIds,
                                    const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
                                    UItemObject* UnInitializedItemObject)
{
	// @gdemers target any actor type referencing this inventory component that may have a backend representation.
	const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on IAVVMResourceProvider::GetProviderUniqueId implementation."),
	                      *Outer->GetName()))
	{
		return INDEX_NONE;
	}

	const TArray<int32> OuterDependencies = UAVVMOnlineBackendUtils::GetElementDependencies(Outer, TargetUniqueId, DataResolverHelper);
	const int32 ItemId = UInventoryUtils::GetObjectUniqueIdentifier(UnInitializedItemObject);

	if (OuterDependencies.IsEmpty() || (ItemId == INDEX_NONE))
	{
		return INDEX_NONE;
	}

	// @gdemers filter the backend set to ensure we dont reallocate an item that was already configured.
	TArray<int32> FilteredSet = OuterDependencies;
	for (const int32 ReservedItemId : NewPrivateIds)
	{
		FilteredSet.Remove(ReservedItemId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = ItemId](const int32 Value)
	{
		// @gdemers decode Value (PrivateItemId) of the backend item, and parse it's type, returning an output value
		// that respect our initial bit encoding defined under AVVMOnlineInventory.h
		const int32 OutValue = UInventoryUtils::DecodeItem(Value);
		return !!(OutValue & SearchId);
	});

	if (ensureAlwaysMsgf(SearchResult != nullptr, TEXT("Couldnt retrieve the ItemId.")))
	{
		// @gdemers your backend private id that represent the allocated UItemObject.
		const int32 PrivateItemId = (*SearchResult);
		const int32 ItemCount = UItemObjectUtils::GetItemStartupStackCount(UnInitializedItemObject, PrivateItemId);
		const int32 StorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE,GET_STORAGE_ID_ENCODING_RSHIFT);
		const int32 StoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE,GET_ITEM_POSITION_ENCODING_RSHIFT);
		UnInitializedItemObject->ModifyRuntimeStackCount(ItemCount);
		UnInitializedItemObject->ModifyRuntimeStorageId(StorageId);
		UnInitializedItemObject->ModifyRuntimeStoragePosition(StoragePosition);
		UnInitializedItemObject->PrivateItemId = PrivateItemId;

		return PrivateItemId;
	}
	else
	{
		return INDEX_NONE;
	}
}

void UItemObjectUtils::RuntimeDestroy(UItemObject* PendingDestroyItemObject)
{
	if (!IsValid(PendingDestroyItemObject))
	{
		return;
	}

	AActor* ChildActor = PendingDestroyItemObject->RuntimeItemActor;
	if (IsValid(ChildActor))
	{
		// @gdemers allow possible pooling of actor if inventory manager system
		// is overloaded.
		UInventoryManagerSubsystem::Static_Shutdown(PendingDestroyItemObject->GetWorld(), ChildActor);
	}

	auto* OwningInventory = PendingDestroyItemObject->GetTypedOuter<UActorInventoryComponent>();
	if (IsValid(OwningInventory))
	{
		// @gdemers little hack that violate encapsulation, but I prefer that than exposing yet another
		// interface to the inventory component class who would allow external removal of an item.
		MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, Items, OwningInventory);
		OwningInventory->RemoveReplicatedSubObject(PendingDestroyItemObject);
		OwningInventory->Items.Remove(PendingDestroyItemObject);
	}
}

void UItemObjectUtils::Insert(const FInsertionContextArgs& Params,
                              UItemObject* PendingInsertItemObject)
{
	// TODO @gdemers Make impl.
}

void UItemObjectUtils::NullifyStorage(UItemObject* PendingDropItemObject)
{
	if (IsValid(PendingDropItemObject))
	{
		const int32 ItemId = UInventoryUtils::GetObjectUniqueIdentifier(PendingDropItemObject);
		PendingDropItemObject->PrivateItemId &= ItemId;
	}
}

void UItemObjectUtils::QualifyStorage(const UActorInventoryComponent* InventoryComponent,
                                      const FStorageQualifierContextArgs& Params,
                                      UItemObject* PendingPickupItemObject)
{
	int32 OutMin_StoragePosition = INT_MAX;
	int32 OutMin_StorageId = INT_MAX;

	struct FStorageSlots
	{
		TArray<int32> Slots;
	};

	TMap<int32/*StorageId*/, FStorageSlots/*OccupiedEntries*/> Storages;
	for (const int32 PrivateItemId : Params.PrivateItemIds)
	{
		const int32 StoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE,GET_ITEM_POSITION_ENCODING_RSHIFT);
		const int32 StorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE,GET_STORAGE_ID_ENCODING_RSHIFT);
		FStorageSlots& OutResult = Storages.FindOrAdd(StorageId);
		OutResult.Slots.Add(StoragePosition);
	}

	const auto GetLowestBound = [](const TWeakObjectPtr<const UActorInventoryComponent>& NewInventoryComponent,
	                               const TMap<int32/*StorageId*/, FStorageSlots/*OccupiedEntries*/>& NewStorages,
	                               int32& OutStoragePosition,
	                               int32& OutStorageId)
	{
		for (auto& [StorageId, StoragePositions] : NewStorages)
		{
			FStorageContextArgs SearchParams;
			SearchParams.InventoryComponent = NewInventoryComponent;
			SearchParams.StartPosition = NULL;
			SearchParams.StorageId = StorageId;
			SearchParams.StoragePositions = StoragePositions.Slots/*OccupiedEntries*/;

			int32 OutSearchResult_StoragePosition = INT_MAX;
			int32 OutSearchResult_StorageId = INT_MAX;
			UItemObjectUtils::GetFreeStorageFromPosition(SearchParams, OutSearchResult_StoragePosition, OutSearchResult_StorageId);

			if (OutSearchResult_StorageId < OutStorageId)
			{
				OutStoragePosition = OutSearchResult_StoragePosition;
				OutStorageId = OutSearchResult_StorageId;
			}
		}
	};

	// @gdemers item was picked up from world, and wasn't stacked on top of existing entry.
	const bool bDoesContains = Storages.Contains(Params.CurrentStorageId/*INDEX_NONE or otherwise*/);
	if (!bDoesContains)
	{
		GetLowestBound(InventoryComponent, Storages, OutMin_StoragePosition, OutMin_StorageId);
	}
	else
	{
		FStorageContextArgs SearchParams;
		SearchParams.InventoryComponent = InventoryComponent;
		SearchParams.StartPosition = Params.CurrentStoragePosition/*INDEX_NONE or otherwise*/;
		SearchParams.StorageId = Params.CurrentStorageId;
		SearchParams.StoragePositions = Storages[Params.CurrentStorageId].Slots;

		int32 OutSearchResult_StoragePosition = INT_MAX;
		int32 OutSearchResult_StorageId = INT_MAX;

		// @gdemers attempting to neighbor the item that generated a stack overflow.
		const bool bCouldPlaceWithinSameStorage = UItemObjectUtils::GetFreeStorageFromPosition(SearchParams, OutSearchResult_StoragePosition, OutSearchResult_StorageId);

		// @gdemers search failed, we fall back to finding the entry sitting at the lower bounds of the storage system.
		if (!bCouldPlaceWithinSameStorage)
		{
			GetLowestBound(InventoryComponent, Storages, OutMin_StoragePosition, OutMin_StorageId);
		}
		else
		{
			OutMin_StoragePosition = OutSearchResult_StoragePosition;
			OutMin_StorageId = OutSearchResult_StorageId;
		}
	}

	if (IsValid(PendingPickupItemObject))
	{
		const int32 StorageId_Shifted = (OutMin_StorageId << GET_STORAGE_ID_ENCODING_RSHIFT/*bit shift is mirrored here*/);
		const int32 StoragePosition_Shifted = (OutMin_StoragePosition << GET_ITEM_POSITION_ENCODING_RSHIFT/*same here*/);
		PendingPickupItemObject->PrivateItemId |= (StorageId_Shifted | StoragePosition_Shifted);
	}
}

bool UItemObjectUtils::GetFreeStorageFromPosition(const FStorageContextArgs& Params,
                                                  int32& OutStoragePosition,
                                                  int32& OutStorageId)
{
	const int32 StorageMaxCapacity = UItemObjectUtils::GetStorageMaxCapacity(Params.InventoryComponent.Get(), Params.StorageId);
	if (!ensureAlwaysMsgf((Params.StoragePositions.Num() <= StorageMaxCapacity) && (StorageMaxCapacity != INDEX_NONE),
	                      TEXT("Allocated number of positions exceed the expectation defined in data for this Storage Id.")))
	{
		return false;
	}

	TArray<int32> Temp;
	Temp.Reserve(StorageMaxCapacity);

	// @gdemers place already allocated space within set.
	for (int32 StoragePosition : Params.StoragePositions)
	{
		Temp[StoragePosition/*may want to not validate position, and crash if ever its poorly encoded.*/] = 1;
	}

	int32 StorageBounds = StorageMaxCapacity;
	int32 SearchIndex = FMath::Clamp(Params.StartPosition, 0, StorageMaxCapacity);

	while ((SearchIndex < StorageBounds) && !!Temp[SearchIndex])
	{
		SearchIndex = ((SearchIndex + 1) % StorageMaxCapacity);
		if (false == !!SearchIndex)
		{
			StorageBounds = FMath::Clamp(Params.StartPosition, 0, StorageMaxCapacity);
		}
	}

	if (SearchIndex != StorageMaxCapacity)
	{
		OutStoragePosition = SearchIndex;
		OutStorageId = Params.StorageId;
		return true;
	}

	return false;
}

bool UItemObjectUtils::HasStorageReachMaxCapacity(const UActorInventoryComponent* InventoryComponent,
                                                  const int32 StorageId,
                                                  const int32 Count)
{
	// @gdemers our count has reach our encoding hard limit. this 100% imply that the storage is full.
	// if theres an overflow from what design configured in DataAsset, theres a problem at the user level!
	const bool bDoesCountReachMaxCapacity = (Count >= (1 << GET_ITEM_POSITION_ENCODING_BIT_RANGE));
	if (bDoesCountReachMaxCapacity)
	{
		return true;
	}

	const int32 StorageMaxCapacity = UItemObjectUtils::GetStorageMaxCapacity(InventoryComponent, StorageId);
	if (!ensureAlwaysMsgf((StorageMaxCapacity != INDEX_NONE),
	                      TEXT("StorageId provided couldn't resolve a valid Storage Capacity.")))
	{
		return false;
	}

	return (Count >= StorageMaxCapacity);
}

int32 UItemObjectUtils::GetStorageMaxCapacity(const UActorInventoryComponent* InventoryComponent,
                                              const int32 StorageId)
{
	constexpr int32 StorageId_Offset = (1 << GET_ITEM_POSITION_ENCODING_RSHIFT);
	const int32 Real_StorageId = (StorageId + StorageId_Offset);

	// Remember that storage are UItemObject, and as such, they hold a tag to a capacity which refer to their max stack_cout, i.e
	// the total of items they can fit in.
	const TObjectPtr<UItemObject>* SearchResult = InventoryComponent->Items.FindByPredicate([SearchId = Real_StorageId](const UItemObject* ItemObject)
	{
		if (!IsValid(ItemObject))
		{
			return false;
		}
		else
		{
			// @gdemers this PrivateItemId holds information about storage but isnt an Item, or an Attachment.
			const bool bHoldStorageId = (ItemObject->PrivateItemId & SearchId);
			const int32 ItemId = UAVVMOnlineEncodingUtils::DecodeInt32(ItemObject->PrivateItemId, GET_ITEM_ID_ENCODING_BIT_RANGE, GET_ITEM_ID_ENCODING_RSHIFT);
			const int32 AttachmentId = UAVVMOnlineEncodingUtils::DecodeInt32(ItemObject->PrivateItemId, GET_ATTACHMENT_ID_ENCODING_BIT_RANGE, GET_ATTACHMENT_ID_ENCODING_RSHIFT);
			return (!!(ItemId & CHECK_CHARACTER_DEPENDENT_ENCODING)) && (false == !!AttachmentId) && (bHoldStorageId);
		}
	});

	if ((SearchResult != nullptr) && IsValid(*SearchResult))
	{
		// @gdemers the backend returned the UItemObject representing the Storage object, this Stack Count represent that number of entries
		// allowed within the storage.
		return (*SearchResult)->GetMaxStackCount();
	}

	return INDEX_NONE;
}

int32 UItemObjectUtils::GetMaxStackCount(const UDataTable* MaxStackCountDataTable,
                                         const FGameplayTag& MaxStackCountTag)
{
	if (!IsValid(MaxStackCountDataTable))
	{
		return INDEX_NONE;
	}

	const FName RowName = UInventorySettings::GetItemMaxStackCount(MaxStackCountTag);
	const auto* SearchResult = MaxStackCountDataTable->FindRow<FItemStackTableRow>(RowName, TEXT(""));
	if (ensureAlwaysMsgf(SearchResult != nullptr,
	                     TEXT("Invalid entry. Cannot find UItemObject entry in the data table.")))
	{
		return SearchResult->MaxStackCount;
	}

	return INDEX_NONE;
}

int32 UItemObjectUtils::GetItemStartupStackCount(const UItemObject* UnInitializedItemObject,
                                                 const int32 ElementId)
{
	if (!IsValid(UnInitializedItemObject) || (ElementId == INDEX_NONE))
	{
		return INDEX_NONE;
	}

	static const auto StackableTagContainer = FGameplayTagContainer(TAG_INVENTORY_ITEM_STACKABLE);
	const bool bDoesItemStack = UnInitializedItemObject->DoesBehaviourHasPartialMatch(StackableTagContainer);
	if (!bDoesItemStack)
	{
		static constexpr int32 One = 1;
		return One;
	}

	const int32 StackCount = UAVVMOnlineEncodingUtils::DecodeInt32(ElementId, GET_ITEM_COUNT_ENCODING_BIT_RANGE,GET_ITEM_COUNT_ENCODING_RSHIFT);
	return StackCount;
}

int32 UItemObjectUtils::MakeRuntimePrivateItemId(const UItemObject* ItemObject)
{
	if (!IsValid(ItemObject))
	{
		return INDEX_NONE;
	}

	const int32 StackCount = ItemObject->GetRuntimeCount() + (1 << GET_ITEM_COUNT_ENCODING_RSHIFT);
	const int32 StorageId = ItemObject->GetStorageId() + (1 << GET_STORAGE_ID_ENCODING_RSHIFT);
	const int32 StoragePosition = ItemObject->GetStoragePosition() + (1 << GET_ITEM_POSITION_ENCODING_RSHIFT);

	// TODO @gdemers theres a problem here !
	// MakeRuntimePrivateItemId is called from within CheckBackend, and indirectly from OnDrop, and OnPickup. This means that our inventory
	// updates in those two cases. We may have an Actor in World representing this UItemObject, or not. If our UItemObject is an attachment, its dependent on another
	// actor being dropped, but also being picked up. Attachment can be drop as a unique element, or parented by an outer. The encoding as to reflect all those cases, and
	// update accordingly.
	const int32 ItemId = UAVVMOnlineEncodingUtils::FilterInt32(ItemObject->PrivateItemId, GET_ITEM_ID_ENCODING_BIT_RANGE, GET_ITEM_ID_ENCODING_RSHIFT);
	const int32 AttachmentId = UAVVMOnlineEncodingUtils::FilterInt32(ItemObject->PrivateItemId, GET_ATTACHMENT_ID_ENCODING_BIT_RANGE, GET_ATTACHMENT_ID_ENCODING_RSHIFT);

	return (ItemId + StackCount + StorageId + StoragePosition + AttachmentId);
}

int32 UItemObjectUtils::GetNumSplits(const UItemObject* SrcItem)
{
	if (!IsValid(SrcItem))
	{
		return INDEX_NONE;
	}

	const int32 MaxCount = SrcItem->GetMaxStackCount();
	const int32 SrcCount = SrcItem->GetRuntimeCount();
	return FMath::CeilToInt(SrcCount * (1.f / MaxCount));
}

UItemObject* UItemObjectUtils::SplitObject(UObject* Outer, UItemObject* SrcItem)
{
	if (!IsValid(SrcItem))
	{
		return nullptr;
	}

	UItemObject* OutItem = NewObject<UItemObject>(Outer, SrcItem->GetFName(), RF_NoFlags, SrcItem);
	if (IsValid(OutItem))
	{
		const int32 MaxCount = SrcItem->GetMaxStackCount();
		const int32 SrcCount = SrcItem->GetRuntimeCount();
		const int32 SplitResources = FMath::Min(SrcCount, MaxCount);
		SrcItem->ModifyRuntimeStackCount(SrcCount - SplitResources);
		OutItem->ModifyRuntimeStackCount(SplitResources);
		// @gdemers storage is qualified next. this shouldnt be transient between instance. only id should be!
		OutItem->PrivateItemId = UAVVMOnlineEncodingUtils::DecodeInt32(SrcItem->PrivateItemId, GET_ITEM_ID_ENCODING_BIT_RANGE, GET_ITEM_ID_ENCODING_RSHIFT);
	}

	return OutItem;
}

AActor* UItemObjectUtils::SpawnWorldItemActor(const UWorld* World, UItemObject* SrcItem)
{
	auto* Out = Cast<APickupActor>(UInventoryManagerSubsystem::Static_CreateItemActor(World, APickupActor::StaticClass(), nullptr));
	if (IsValid(Out))
	{
		Out->Setup(SrcItem);
	}

	return Out;
}

void UItemObjectUtils::DestroyWorldItemActor(const UItemObject* SrcItem)
{
	if (!IsValid(SrcItem))
	{
		return;
	}

	auto* OwningOuter = SrcItem->GetTypedOuter<APickupActor>();
	if (IsValid(OwningOuter))
	{
		// @gdemers invalidate our Pickup Actor to prevent collision during pooling, if supported.
		OwningOuter->Setup(nullptr);
		// @gdemers allow possible pooling of actor if inventory manager system
		// is overloaded.
		UInventoryManagerSubsystem::Static_Shutdown(OwningOuter->GetWorld(), OwningOuter);
	}
}
