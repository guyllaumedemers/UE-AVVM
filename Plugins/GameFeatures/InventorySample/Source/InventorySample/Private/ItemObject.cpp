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

#include "AVVMGameplayUtils.h"
#include "AVVMSocketTargetingHelper.h"
#include "InventoryManagerSubsystem.h"
#include "InventorySample.h"
#include "InventorySettings.h"
#include "PickupActor.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/ItemStackTableRow.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Net/UnrealNetwork.h"

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

void UItemObject::ModifyRuntimeCount(const int32 NewCountModifier)
{
	RuntimeItemState.Counter = FMath::Clamp<int32>((RuntimeItemState.Counter + NewCountModifier), 0, 999);
	MARK_PROPERTY_DIRTY_FROM_NAME(UItemObject, RuntimeItemState, this);

	OnRep_ItemStateModified(RuntimeItemState);

	if (false == !!RuntimeItemState.Counter)
	{
		const FGameplayTagContainer BlockingTags = UInventorySettings::GetEmptyItemCount_BlockedActions();
		ModifyRuntimeState(BlockingTags, {});
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
	return (false == !!RuntimeItemState.Counter);
}

bool UItemObject::CanStack(const UItemObject* Item) const
{
	// @gdemers validate if both items are of same types.
	if (!IsValid(Item) || (Item->GetItemActorId() != GetItemActorId()))
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
	Item->ModifyRuntimeCount((TotalStackCount - ClampedStackCount)/*handle left-over*/);
	ModifyRuntimeCount(ClampedStackCount);

	const bool bDoesStackOverflow = (TotalStackCount > ClampedStackCount);
	return bDoesStackOverflow;
}

int32 UItemObject::GetMaxStackCount() const
{
	// @gdemers shouldnt require async loading as this table will ever only contains integers.
	const TSoftObjectPtr<UDataTable>& DataTable = UInventorySettings::GetItemMaxStackCountDataTable();
	const UDataTable* MaxCountDataTable = DataTable.LoadSynchronous();
	return UItemObjectUtils::GetMaxStackCount(MaxCountDataTable, MaxStackCount_CategoryTag);
}

int32 UItemObject::GetRuntimeCount() const
{
	return RuntimeItemState.Counter;
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

	const bool bHasDifferentCount = (RuntimeItemState.Counter != OldItemState.Counter);
	if (bHasDifferentCount)
	{
		OnItemRuntimeCountChanged.Broadcast(RuntimeItemState.Counter);
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

int32 UItemObjectUtils::GetMaxStackCount(const UDataTable* MaxStackCountDataTable,
                                         const FGameplayTag& MaxStackCountTag)
{
	if (!IsValid(MaxStackCountDataTable))
	{
		return INDEX_NONE;
	}

	const FName RowName = UInventorySettings::GetItemMaxStackCount(MaxStackCountTag);
	const auto* SearchResult = MaxStackCountDataTable->FindRow<FItemStackTableRow>(RowName, TEXT(""));
	if (!ensureAlwaysMsgf(SearchResult != nullptr,
	                      TEXT("Invalid entry. Cannot find UItemObject entry in the data table.")))
	{
		return INDEX_NONE;
	}

	const int32 MaxStackCount = SearchResult->MaxStackCount;
	return MaxStackCount;
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
		SrcItem->ModifyRuntimeCount(SrcCount - SplitResources);
		OutItem->ModifyRuntimeCount(SplitResources);
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
		OwningOuter->Destroy();
	}
}
