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
#include "InventoryManagerSubsystem.h"
#include "InventorySample.h"
#include "InventorySettings.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void UItemObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemObject, RuntimeItemActor);
	DOREPLIFETIME(UItemObject, RuntimeItemState);
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
	OnRep_ItemStateModified(RuntimeItemState);
}

void UItemObject::ModifyRuntimeCount(const int32 NewCountModifier)
{
	RuntimeItemState.Counter = FMath::Clamp<int32>((RuntimeItemState.Counter + NewCountModifier), 0, 999);
	OnRep_ItemStateModified(RuntimeItemState);
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

int32 UItemObject::GetRuntimeCount() const
{
	return RuntimeItemState.Counter;
}

const FDataRegistryId& UItemObject::GetItemActorId() const
{
	return ItemActorId;
}

const FDataRegistryId& UItemObject::GetItemProgressionId() const
{
	return ItemProgressionId;
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
	OnRequestItemActorClassComplete.BindUObject(this, &UItemObject::OnSoftObjectAcquired, Callback, ActorDefinitionDataAsset->GetActorAttributeSetSoftObjectPath());
	ItemActorHandle = UAssetManager::Get().LoadAssetList({ActorDefinitionDataAsset->GetActorSoftObjectPath()}, OnRequestItemActorClassComplete);
}

AActor* UItemObject::SpawnActorClass(const UClass* NewActorClass, AActor* NewAnchor)
{
	if (!IsValid(NewActorClass))
	{
		return nullptr;
	}

	RuntimeItemActor = UInventoryManagerSubsystem::Static_CreateItemActor(GetWorld(), NewActorClass, NewAnchor);

	const bool bShouldSpawnAndAttach = RuntimeItemState.StateTags.HasAllExact(FGameplayTagContainer{UInventorySettings::GetEquippedTag()});
	ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetInstancedTag()}, FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()});

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Executed from \"%s\". Adding New Visual Actor \"%s\" on Item \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(NewAnchor).GetData(),
	       *RuntimeItemActor->GetName(),
	       *GetName());

	if (bShouldSpawnAndAttach && ensureAlwaysMsgf(IsValid(RuntimeItemActor),
	                                              TEXT("Item Actor Class Failed to create an instance in World!")))
	{
		RuntimeItemActor->AttachToActor(NewAnchor, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	}

	return RuntimeItemActor;
}

void UItemObject::OnSoftObjectAcquired(FOnRequestItemActorClassComplete Callback, const FSoftObjectPath NewActorAttributeSetSoftObjectPath)
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
