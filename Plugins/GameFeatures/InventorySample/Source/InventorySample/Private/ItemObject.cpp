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
#include "AVVMLogger.h"
#include "AVVMSocketTargetingHelper.h"
#include "AVVMToolkitUtils.h"
#include "InventoryFileHelper.h"
#include "InventoryManagerSubsystem.h"
#include "InventorySampleModule.h"
#include "InventorySettings.h"
#include "InventoryUtils.h"
#include "PickupActor.h"
#include "TimerManager.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/ItemStackTableRow.h"
#include "Engine/AssetManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Resources/AVVMResourceProvider.h"
#include "Tags/PrivateTags.h"

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

#if WITH_EDITOR
void UItemObject::MoveDataToSparseClassDataStruct() const
{
	// make sure we don't overwrite the sparse data if it has been saved already
	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass == nullptr || BPClass->bIsSparseClassDataSerializable == true)
	{
		return;
	}

	Super::MoveDataToSparseClassDataStruct();

#if WITH_EDITORONLY_DATA
	// Unreal Header Tool (UHT) will create GetMySparseClassData automatically.
	FItemSparseData* SparseClassData = GetMutableItemSparseData();

	// Modify these lines to include all Sparse Class Data properties.
	SparseClassData->ItemBehaviourTypeTags = ItemBehaviourTypeTags_DEPRECATED;
	SparseClassData->ItemTypeTags = ItemTypeTags_DEPRECATED;
	SparseClassData->ItemSlotTags = ItemSlotTags_DEPRECATED;
	SparseClassData->ItemAttachmentSlotTags = ItemAttachmentSlotTags_DEPRECATED;
	SparseClassData->MaxStackCount_CategoryTag = MaxStackCount_CategoryTag_DEPRECATED;
	SparseClassData->ItemActorId = ItemActorId_DEPRECATED;
	SparseClassData->ItemActorUIId = ItemActorUIId_DEPRECATED;
	SparseClassData->SocketName = SocketName_DEPRECATED;
#endif // WITH_EDITORONLY_DATA
}
#endif // WITH_EDITOR

void UItemObject::ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags)
{
	const FItemState OldState = RuntimeItemState;
	
	RuntimeItemState.StateTags.RemoveTags(RemovedTags);
	RuntimeItemState.StateTags.AppendTags(AddedTags);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(OldState);
}

void UItemObject::ModifyRuntimeSlotTag(const FGameplayTag& NewSlotTag)
{
	const FItemState OldState = RuntimeItemState;
	
	RuntimeItemState.ActiveSlotTag = NewSlotTag;
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(OldState);
}

void UItemObject::ModifyRuntimeStorageId(const int32 NewStorageId)
{
	const FItemState OldState = RuntimeItemState;
	
	static const int32 MaxStorageId = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_STORAGE_ID_ENCODING_BIT_RANGE);
	RuntimeItemState.StorageId = FMath::Clamp<int32>(NewStorageId, 0, MaxStorageId);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(OldState);
}

void UItemObject::ModifyRuntimeStoragePosition(const int32 NewStoragePosition)
{
	const FItemState OldState = RuntimeItemState;
	
	const int32 MaxStorageCapacity = GetRuntimeStorageMaxCapacity();
	RuntimeItemState.StoragePosition = FMath::Clamp<int32>(NewStoragePosition, 0, MaxStorageCapacity);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(OldState);
}

void UItemObject::ModifyRuntimeStackCount(const int32 NewStackCount, const bool bShouldAlwaysClamp/*Mostly bypassed during Object Init phase*/)
{
	const FItemState OldState = RuntimeItemState;
	
	const int32 MaxStackCount = GetMaxStackCount();
	RuntimeItemState.StackCount = bShouldAlwaysClamp ? FMath::Clamp<int32>(NewStackCount, 0, MaxStackCount) : NewStackCount;
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(OldState);

	// @gdemers early out on non-empty stack.
	if (!!RuntimeItemState.StackCount)
	{
		return;
	}

	// @gdemers check if we are meeting design conditions for destroying this item from inventory.
	static const auto DestroyConditions = FGameplayTagContainer(TAG_INVENTORYSAMPLE_ITEM_BEHAVIOUR_DESTROY_WHEN_EMPTY);
	const bool bShouldDestroyOnEmptyStack = DoesBehaviourHasPartialMatch(DestroyConditions);

	// @gdemers invalidate any actions we could be executing while pending to destroy, or while the stack is empty.
	const FGameplayTagContainer& BlockingTags = UInventorySettings::GetBlockingTagsWhenEmpty();
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
	return Compare.HasAnyExact(GetItemTypeTags());
}

bool UItemObject::DoesTypeHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(GetItemTypeTags());
}

bool UItemObject::DoesBehaviourHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(GetItemBehaviourTypeTags());
}

bool UItemObject::DoesBehaviourHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(GetItemBehaviourTypeTags());
}

bool UItemObject::DoesAttachmentSlotHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(GetItemAttachmentSlotTags());
}

bool UItemObject::DoesAttachmentSlotHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(GetItemAttachmentSlotTags());
}

bool UItemObject::DoesSlotHasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(GetItemSlotTags());
}

bool UItemObject::DoesSlotHasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(GetItemSlotTags());
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
	if (!IsValid(Item) || (Item->BP_GetItemActorId() != BP_GetItemActorId()))
	{
		return false;
	}

	static const auto StackableTagContainer = FGameplayTagContainer(TAG_INVENTORYSAMPLE_ITEM_BEHAVIOUR_CAN_STACK);
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

bool UItemObject::Stack(UItemObject* Src)
{
	if (!IsValid(Src))
	{
		return false;
	}

	// @gdemers get stack count from sum between both objects.
	const int32 TotalStackCount = (Src->GetRuntimeCount() + GetRuntimeCount());
	const int32 ClampedStackCount = FMath::Clamp(TotalStackCount, 0, GetMaxStackCount());

	// @gdemers update stack internal representation. Note : the function input may overflow the stack
	// 'this' UItemObject reference.
	Src->ModifyRuntimeStackCount((TotalStackCount - ClampedStackCount)/*handle left-over*/);
	ModifyRuntimeStackCount(ClampedStackCount);

	const bool bDoesStackOverflow = (TotalStackCount > ClampedStackCount);
	return bDoesStackOverflow;
}

const FGameplayTag& UItemObject::GetRuntimeItemSlotTag() const
{
	return RuntimeItemState.ActiveSlotTag;
}

int32 UItemObject::GetRuntimeCount() const
{
	return RuntimeItemState.StackCount;
}

int32 UItemObject::GetRuntimeStorageId() const
{
	return RuntimeItemState.StorageId;
}

int32 UItemObject::GetRuntimeStoragePosition() const
{
	return RuntimeItemState.StoragePosition;
}

int32 UItemObject::GetRuntimeStorageMaxCapacity() const
{
	const int32 StorageMaxCapacity = UItemObjectUtils::GetStorageMaxCapacity(GetTypedOuter<UActorInventoryComponent>(), RuntimeItemState.StorageId);
	if (!ensureAlwaysMsgf((StorageMaxCapacity != INDEX_NONE),
	                      TEXT("StorageId provided couldn't resolve a valid Storage Capacity.")))
	{
		return INDEX_NONE;
	}
	else
	{
		return StorageMaxCapacity;
	}
}

int32 UItemObject::GetMaxStackCount() const
{
	// @gdemers shouldnt require async loading as this table will ever only contains integers.
	const TSoftObjectPtr<UDataTable>& DataTable = UInventorySettings::GetItemMaxStackCountDataTable();
	const UDataTable* MaxCountDataTable = DataTable.LoadSynchronous();
	if (!ensureAlwaysMsgf(IsValid(MaxCountDataTable), TEXT("Missing valid StackCount Data Table in project Settings.")))
	{
		return INDEX_NONE;
	}

	const bool bIsStorage = DoesTypeHasPartialMatch(FGameplayTagContainer(TAG_INVENTORYSAMPLE_ITEM_TYPE_STORAGE));
	if (bIsStorage)
	{
		static const int32 MaxStorageCapacityBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_POSITION_ENCODING_BIT_RANGE);
		const int32 MaxStackCount = UItemObjectUtils::GetMaxStackCount(MaxCountDataTable, GetMaxStackCount_CategoryTag());
		return FMath::Clamp(MaxStackCount, 0, MaxStorageCapacityBounds);
	}

	static const auto StackableTagContainer = FGameplayTagContainer(TAG_INVENTORYSAMPLE_ITEM_BEHAVIOUR_CAN_STACK);
	const bool bDoesStack = DoesBehaviourHasPartialMatch(StackableTagContainer);
	if (!bDoesStack)
	{
		static constexpr int32 One = 1;
		return One;
	}
	else
	{
		static const int32 MaxStackCountBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_COUNT_ENCODING_BIT_RANGE);
		const int32 MaxStackCount = UItemObjectUtils::GetMaxStackCount(MaxCountDataTable, GetMaxStackCount_CategoryTag());
		return FMath::Clamp(MaxStackCount, 0, MaxStackCountBounds);
	}
}

const FGameplayTagContainer& UItemObject::BP_GetItemSlotTags() const
{
	return GetItemSlotTags();
}

const FDataRegistryId& UItemObject::BP_GetItemActorId() const
{
	return GetItemActorId();
}

const FDataRegistryId& UItemObject::BP_GetItemActorUIId() const
{
	return GetItemActorUIId();
}

void UItemObject::GetItemActorClassAsync(const UObject* NewActorDefinitionDataAsset,
                                         const FOnRequestItemActorClassComplete& Callback)
{
	const auto* ActorDefinitionDataAsset = Cast<UAVVMActorDefinitionDataAsset>(NewActorDefinitionDataAsset);
	if (!IsValid(ActorDefinitionDataAsset))
	{
		return;
	}

	ModifyRuntimeState(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_PENDING_SPAWN}, {});

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

	const bool bShouldSpawnAndAttach = RuntimeItemState.StateTags.HasAllExact(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_EQUIPPED});
	ModifyRuntimeState(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_INSTANCED}, FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_STATE_PENDING_SPAWN});
	AVVM_LOGGER_LOG(LogInventorySample,
	                Outer,
	                Outer,
	                TEXT("Adding New Visual Actor %s on item %s"),
	                *GetNameSafe(RuntimeItemActor),
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
		Params.SocketName = GetSocketName();
		// TODO @gdemers : will require proper tag handling later. keep gameplay tag container for now until requirements
		// changes.
		Params.AttachmentSlotTag = GetItemAttachmentSlotTags().First();
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
	const bool bDoesDefineAttachmentSlot = DoesAttachmentSlotHasPartialMatch(FGameplayTagContainer(NewItemAttachmentSlotTag));
	if (ensureAlwaysMsgf(bDoesDefineAttachmentSlot,
	                     TEXT("Attachment Slot Tag \"%s\" isn't expected to be referenced on the UItemObject targeted \"%s\"."),
	                     *NewItemAttachmentSlotTag.GetTagName().ToString(),
	                     *GetName()))
	{
		NonReplicatedItemAttachmentActors.FindOrAdd(NewItemAttachmentSlotTag, NewAttachment);
	}
}

void UItemObject::OnNewSocketItemDetached(const FGameplayTag& NewItemAttachmentSlotTag)
{
	const bool bDoesDefineAttachmentSlot = DoesAttachmentSlotHasPartialMatch(FGameplayTagContainer(NewItemAttachmentSlotTag));
	if (ensureAlwaysMsgf(bDoesDefineAttachmentSlot,
	                     TEXT("Attachment Slot Tag \"%s\" isn't expected to be referenced on the UItemObject targeted \"%s\"."),
	                     *NewItemAttachmentSlotTag.GetTagName().ToString(),
	                     *GetName()))
	{
		NonReplicatedItemAttachmentActors.Remove(NewItemAttachmentSlotTag);
	}
}

int32 UItemObjectUtils::RuntimeInitStaticItem(const UObject* Outer,
                                              const TArray<int32>& NewPrivateIds,
                                              UItemObject* UnInitializedItemObject)
{
	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMResourceProvider>(Outer);
	if (!bResult)
	{
		return INDEX_NONE;
	}

	const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on IAVVMResourceProvider::GetProviderUniqueId implementation."),
	                      *Outer->GetName()))
	{
		return INDEX_NONE;
	}

	const int32 ItemId = UInventoryUtils::GetObjectUniqueIdentifier(UnInitializedItemObject);
	if (!ensureAlwaysMsgf(ItemId != INDEX_NONE,
	                      TEXT("Couldn't retrieve a valid ItemId. Are you missing a valid FDataRegistryId reference within this Object Class definition ?")))
	{
		return INDEX_NONE;
	}

	// @gdemers get-set file from disk caching all inventory providers representation.
	const FStringView FileContent = UInventoryFileHelper::Static_GetSetFileContent();

	// @gdemers fetch provider payload from disk representation.
	const FString InventoryProviderPayload = UInventoryUtils::GetInventoryProviderById(FileContent.GetData(), TargetUniqueId);
	if (InventoryProviderPayload.IsEmpty())
	{
		return INDEX_NONE;
	}

	// @gdemers read private item id from payload.
	const int32 PrivateItemId = UInventoryUtils::GetItemPrivateId(InventoryProviderPayload, NewPrivateIds, ItemId);
	const int32 ItemCount = UItemObjectUtils::GetItemStartupStackCount(UnInitializedItemObject, PrivateItemId);
	const int32 StorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE,GET_STORAGE_ID_ENCODING_RSHIFT);
	const int32 StoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE,GET_ITEM_POSITION_ENCODING_RSHIFT);
	const FGameplayTag SlotTag = UInventoryUtils::GetItemSlotTagFromPayload(InventoryProviderPayload, PrivateItemId);
	UnInitializedItemObject->ModifyRuntimeStackCount(ItemCount);
	UnInitializedItemObject->ModifyRuntimeStorageId(StorageId);
	UnInitializedItemObject->ModifyRuntimeStoragePosition(StoragePosition);
	UnInitializedItemObject->ModifyRuntimeSlotTag(SlotTag);
	UnInitializedItemObject->PrivateItemId = PrivateItemId;
	return PrivateItemId;
}

int32 UItemObjectUtils::RuntimeInitOnlineItem(const UObject* Outer,
                                              const TArray<int32>& NewPrivateIds,
                                              const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
                                              UItemObject* UnInitializedItemObject)
{
	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMResourceProvider>(Outer);
	if (!bResult)
	{
		return INDEX_NONE;
	}
	
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

	if (OuterDependencies.IsEmpty() || !ensureAlwaysMsgf(ItemId != INDEX_NONE,
	                                                     TEXT("Couldn't retrieve a valid ItemId. Are you missing a valid FDataRegistryId reference within this Object Class definition ?")))
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
		// @gdemers filter Value (PrivateItemId) of the backend item, and parse it's type, returning an output value
		// that respect our initial bit encoding defined under AVVMOnlineInventory.h
		const int32 OutValue = UItemObjectUtils::FilterItemPrivateId(Value);
		return (false == (OutValue ^ SearchId))/*if both bits are identical, return 0.*/;
	});

	if (ensureAlwaysMsgf(SearchResult != nullptr, TEXT("Couldn't retrieve the ItemId.")))
	{
		// @gdemers your backend private id that represent the allocated UItemObject.
		const int32 PrivateItemId = (*SearchResult);
		const int32 ItemCount = UItemObjectUtils::GetItemStartupStackCount(UnInitializedItemObject, PrivateItemId);
		const int32 StorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE,GET_STORAGE_ID_ENCODING_RSHIFT);
		const int32 StoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE,GET_ITEM_POSITION_ENCODING_RSHIFT);
		const FGameplayTag SlotTag = UInventoryUtils::GetItemSlotTag(Outer, TargetUniqueId, PrivateItemId);
		UnInitializedItemObject->ModifyRuntimeStackCount(ItemCount);
		UnInitializedItemObject->ModifyRuntimeStorageId(StorageId);
		UnInitializedItemObject->ModifyRuntimeStoragePosition(StoragePosition);
		UnInitializedItemObject->ModifyRuntimeSlotTag(SlotTag);
		UnInitializedItemObject->PrivateItemId = PrivateItemId;

		return PrivateItemId;
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 UItemObjectUtils::GetPrivateItemId(const UItemObject* ItemObject)
{
	return IsValid(ItemObject) ? ItemObject->PrivateItemId : INDEX_NONE;
}

int32 UItemObjectUtils::FilterItem(const UItemObject* ItemObject)
{
	if (IsValid(ItemObject))
	{
		// @gdemers return a non-shifted version of the ItemId. Keep in mind that the range of the bit encoding define
		// the item category manipulated!
		return UItemObjectUtils::FilterItemPrivateId(ItemObject->PrivateItemId);
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 UItemObjectUtils::FilterItemPrivateId(const int32 EncodedBits)
{
	int32 BitRange = INDEX_NONE;
	int32 BitShift = INDEX_NONE;

	// @gdemers order dependent since attachment may be tied to an item 
	if (UItemObjectUtils::IsAttachment(EncodedBits))
	{
		BitRange = GET_ATTACHMENT_ID_ENCODING_BIT_RANGE;
		BitShift = GET_ATTACHMENT_ID_ENCODING_RSHIFT;
	}
	else if (UItemObjectUtils::IsStorage(EncodedBits))
	{
		BitRange = GET_STORAGE_ID_ENCODING_BIT_RANGE;
		BitShift = GET_STORAGE_ID_ENCODING_RSHIFT;
	}
	else
	{
		BitRange = GET_ITEM_ID_ENCODING_BIT_RANGE;
		BitShift = GET_ITEM_ID_ENCODING_RSHIFT;
	}

	const int32 NonShiftedItemId = UAVVMOnlineEncodingUtils::FilterInt32(EncodedBits, BitRange, BitShift);
	return NonShiftedItemId;
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
		const int32 StorageIdBitmask = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_STORAGE_ID_ENCODING_BIT_RANGE) << GET_STORAGE_ID_ENCODING_RSHIFT;
		const int32 ItemPositionBitmask = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_POSITION_ENCODING_BIT_RANGE) << GET_ITEM_POSITION_ENCODING_RSHIFT;
		PendingDropItemObject->PrivateItemId &= ~StorageIdBitmask;
		PendingDropItemObject->PrivateItemId &= ~ItemPositionBitmask;
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
	Temp.Init(0, StorageMaxCapacity);

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
	const bool bDoesCountReachMaxCapacity = (Count >= UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_POSITION_ENCODING_BIT_RANGE));
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
	if (!IsValid(InventoryComponent))
	{
		return INDEX_NONE;
	}
	
	const int32 NewStorageId = UAVVMOnlineEncodingUtils::EncodeInt32(StorageId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
	// @gdemers Remember that storage are also UItemObject, and as such, they hold a tag to a capacity which refer to their max stack_count, i.e the total of items they can fit in.
	// IMPORTANT : However, remember that when initializing data from file on disk, our item collection may or may not reference a storage, which means that retrieving its
	// configuration from the runtime representation may be impossible at time, or ever. Example : A shop default inventory configuration set in the Data Table (proj. Settings)
	// require capacity information for default initialization of its item layout, but doesn't care much about instancing the containers.
	const TObjectPtr<UItemObject>* SearchResult = InventoryComponent->Items.FindByPredicate([SearchId = NewStorageId](const UItemObject* ItemObject)
	{
		if (!IsValid(ItemObject))
		{
			return false;
		}
		else
		{
			// @gdemers get the PrivateId, and validate storage item (Not ownership).
			const bool bResult = (false == (UItemObjectUtils::FilterItem(ItemObject)/*NonShiftedItemId*/ ^ SearchId));
			return bResult;
		}
	});

	// @gdemers runtime check on storage capacity.
	if ((SearchResult != nullptr) && IsValid(*SearchResult))
	{
		// @gdemers the backend returned the UItemObject representing the Storage object, this Stack Count represent that number of entries
		// allowed within the storage.
		return (*SearchResult)->GetMaxStackCount();
	}

	// @gdemers offline check (when inventory isnt initialized) on storage capacity.
	const TSoftObjectPtr<UDataTable>& DataTable = UInventorySettings::GetItemMaxStackCountDataTable();
	const UDataTable* MaxCountDataTable = DataTable.LoadSynchronous();
	if (ensureAlwaysMsgf(IsValid(MaxCountDataTable), TEXT("Missing valid StackCount Data Table in project Settings.")))
	{
		static const int32 MaxStorageCapacityBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_POSITION_ENCODING_BIT_RANGE);
		const FGameplayTag& StorageCapacityTag = UInventorySettings::GetStorageCapacityTagById(NewStorageId);
		const int32 MaxStackCount = UItemObjectUtils::GetMaxStackCount(MaxCountDataTable, StorageCapacityTag);
		return FMath::Clamp(MaxStackCount, 0, MaxStorageCapacityBounds);
	}
	else
	{
		return INDEX_NONE;
	}
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

	static const auto StackableTagContainer = FGameplayTagContainer(TAG_INVENTORYSAMPLE_ITEM_BEHAVIOUR_CAN_STACK);
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

	const int32 StackCount = UAVVMOnlineEncodingUtils::EncodeInt32(ItemObject->GetRuntimeCount(),
	                                                               GET_ITEM_COUNT_ENCODING_BIT_RANGE,
	                                                               GET_ITEM_COUNT_ENCODING_RSHIFT);

	const int32 StorageId = UAVVMOnlineEncodingUtils::EncodeInt32(ItemObject->GetRuntimeStorageId(),
	                                                              GET_STORAGE_ID_ENCODING_BIT_RANGE,
	                                                              GET_STORAGE_ID_ENCODING_RSHIFT);

	// TODO @gdemers storage are edge cases, and do not support positioning atm.
	const int32 OldPrivateItemId = UItemObjectUtils::GetPrivateItemId(ItemObject);
	if (UItemObjectUtils::IsStorage(OldPrivateItemId))
	{
		return (StackCount + StorageId);
	}

	// TODO @gdemers theres a problem here !
	// MakeRuntimePrivateItemId is called from within CheckBackend, and indirectly from OnDrop, and OnPickup. This means that our inventory
	// updates in those two cases. We may have an Actor in World representing this UItemObject, or not. If our UItemObject is an attachment, its dependent on another
	// actor being dropped, but also being picked up. Attachment can be drop as a unique element, or parented by an outer. The encoding as to reflect all those cases, and
	// update accordingly.
	const int32 ItemId = UAVVMOnlineEncodingUtils::FilterInt32(ItemObject->PrivateItemId,
	                                                           GET_ITEM_ID_ENCODING_BIT_RANGE,
	                                                           GET_ITEM_ID_ENCODING_RSHIFT);

	const int32 AttachmentId = UAVVMOnlineEncodingUtils::FilterInt32(ItemObject->PrivateItemId,
	                                                                 GET_ATTACHMENT_ID_ENCODING_BIT_RANGE,
	                                                                 GET_ATTACHMENT_ID_ENCODING_RSHIFT);

	const int32 StoragePosition = UAVVMOnlineEncodingUtils::EncodeInt32(ItemObject->GetRuntimeStoragePosition(),
	                                                                    GET_ITEM_POSITION_ENCODING_BIT_RANGE,
	                                                                    GET_ITEM_POSITION_ENCODING_RSHIFT);

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

	// @gdemers could provide Template Object to initialize our new UItemObject but we would have to NOT provide
	// a Name, otherwise, if provided our Src->GetFName(), we would end up invalidating the Src object.
	UItemObject* OutItem = NewObject<UItemObject>(Outer, SrcItem->GetClass());
	if (IsValid(OutItem))
	{
		const int32 MaxCount = SrcItem->GetMaxStackCount();
		const int32 SrcCount = SrcItem->GetRuntimeCount();
		const int32 SplitResources = FMath::Min(SrcCount, MaxCount);
		SrcItem->ModifyRuntimeStackCount(SrcCount - SplitResources);
		OutItem->ModifyRuntimeStackCount(SplitResources);
		OutItem->PrivateItemId = SrcItem->PrivateItemId;
		// @gdemers we need to invalidate storage so we can correctly qualify it next!
		OutItem->PrivateItemId &= ~UAVVMOnlineEncodingUtils::FilterInt32(SrcItem->PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
		OutItem->PrivateItemId &= ~UAVVMOnlineEncodingUtils::FilterInt32(SrcItem->PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE, GET_ITEM_POSITION_ENCODING_RSHIFT);
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

bool UItemObjectUtils::IsItem(const int32 EncodedBits)
{
	const int32 ItemId = UAVVMOnlineEncodingUtils::FilterInt32(EncodedBits,
	                                                           GET_ITEM_ID_ENCODING_BIT_RANGE,
	                                                           GET_ITEM_ID_ENCODING_RSHIFT);

	const bool bResult = !!ItemId;
	return bResult;
}

bool UItemObjectUtils::IsAttachment(const int32 EncodedBits)
{
	const int32 AttachmentId = UAVVMOnlineEncodingUtils::FilterInt32(EncodedBits,
																	 GET_ATTACHMENT_ID_ENCODING_BIT_RANGE,
																	 GET_ATTACHMENT_ID_ENCODING_RSHIFT);

	const bool bResult = !!AttachmentId;
	return bResult;
}

bool UItemObjectUtils::IsStorage(const int32 EncodedBits)
{
	const bool bIsStorage = (!IsItem(EncodedBits) && !IsAttachment(EncodedBits));
	return bIsStorage;
}

UItemObject* UItemObjectUtils::MakeZeroInitItemObject(UObject* Outer)
{
	auto* TestObject = NewObject<UItemObject>(Outer);
	TestObject->PrivateItemId = (UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_ID_ENCODING_BIT_RANGE) - 1);
	return TestObject;
}
