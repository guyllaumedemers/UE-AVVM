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

#include "InventorySettings.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "Resources/AVVMResourceManagerComponent.h"

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
}

bool UItemObject::HasRuntimeState(const FGameplayTagContainer& Compare) const
{
	return RuntimeItemState.StateTags.HasAllExact(Compare);
}

bool UItemObject::HasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return ItemTypeTags.HasAnyExact(Compare);
}

bool UItemObject::HasExactMatch(const FGameplayTagContainer& Compare) const
{
	return ItemTypeTags.HasAllExact(Compare);
}

const FDataRegistryId& UItemObject::GetItemProgressionId() const
{
	return ItemProgressionId;
}

void UItemObject::GetItemActorClassAsync(const FSoftObjectPath& ItemActorSoftObjectPath,
                                         const FOnRequestItemActorClassComplete& Callback)
{
	ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()}, {});

	FStreamableDelegate OnRequestItemActorClassComplete;
	OnRequestItemActorClassComplete.BindUObject(this, &UItemObject::OnSoftObjectAcquired, Callback);
	ItemProgressionStageHandle = UAssetManager::Get().LoadAssetList({ItemActorSoftObjectPath}, OnRequestItemActorClassComplete);
}

void UItemObject::SpawnActorClass(const AActor* Anchor,
                                  const TSoftClassPtr<AActor>& NewActorClass)
{
	if (!NewActorClass.IsValid())
	{
		return;
	}

	const bool bShouldSpawnAndAttach = HasRuntimeState(FGameplayTagContainer{UInventorySettings::GetEquippedTag()});

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const FTransform NewItemTransform = GetSpawningAnchorTransform(Anchor, bShouldSpawnAndAttach);
		RuntimeItemActor = World->SpawnActor(NewActorClass->GetClass(), &NewItemTransform, FActorSpawnParameters());
		ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetInstancedTag()}, FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()});
	}

	if (bShouldSpawnAndAttach && ensureAlwaysMsgf(IsValid(RuntimeItemActor),
	                                              TEXT("Item Actor Class Failed to create an instance in World!")))
	{
		// @gdemers bad practice but avoid code refactoring!
		RuntimeItemActor->AttachToActor(const_cast<AActor*>(Anchor), FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	}
}

void UItemObject::OnSoftObjectAcquired(FOnRequestItemActorClassComplete OnRequestItemActorClassComplete)
{
	if (!ItemProgressionStageHandle.IsValid())
	{
		OnRequestItemActorClassComplete.ExecuteIfBound(nullptr, this);
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	ItemProgressionStageHandle->GetLoadedAssets(OutStreamableAssets);

	if (!OutStreamableAssets.IsEmpty())
	{
		OnRequestItemActorClassComplete.ExecuteIfBound(OutStreamableAssets[0]->GetClass(), this);
	}
	else
	{
		OnRequestItemActorClassComplete.ExecuteIfBound(nullptr, this);
	}
}

FTransform UItemObject::GetSpawningAnchorTransform(const AActor* NewOuter, const bool bShouldAttachToSocket) const
{
	FTransform ItemAnchorTransform = NewOuter->GetActorTransform();
	if (bShouldAttachToSocket)
	{
		const auto* MeshComponent = NewOuter->GetComponentByClass<UMeshComponent>();
		if (IsValid(MeshComponent))
		{
			ItemAnchorTransform = MeshComponent->GetSocketTransform(SocketName);
		}
	}

	return ItemAnchorTransform;
}
