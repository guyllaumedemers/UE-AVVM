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
#include "Data/ItemProgressionDefinitionDataAsset.h"
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

void UItemObject::GetItemActorClassAsync(const UObject* NewProgressionDefinitionData,
                                         const int32 NewProgressionStageIndex,
                                         const FOnRequestItemActorClassComplete& Callback)
{
	const auto* ProgressionDefinitionDataAsset = Cast<UItemProgressionDefinitionDataAsset>(NewProgressionDefinitionData);
	if (!IsValid(ProgressionDefinitionDataAsset))
	{
		return;
	}

	ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()}, {});

	const FSoftObjectPath ProgressionStageSoftObjectPath = ProgressionDefinitionDataAsset->GetProgressionStageItemActorOverride(NewProgressionStageIndex);
	if (ProgressionStageSoftObjectPath.IsValid())
	{
		FStreamableDelegate OnRequestItemActorClassComplete;
		OnRequestItemActorClassComplete.BindUObject(this, &UItemObject::OnProgressionStageAcquired, Callback);
		ItemProgressionStageHandle = UAssetManager::Get().LoadAssetList({ProgressionStageSoftObjectPath}, OnRequestItemActorClassComplete);
	}
	else
	{
		FStreamableDelegate OnRequestItemActorClassComplete;
		OnRequestItemActorClassComplete.BindUObject(this, &UItemObject::OnSoftObjectAcquired, Callback);
		ItemProgressionStageHandle = UAssetManager::Get().LoadAssetList({ProgressionDefinitionDataAsset->GetDefaultItemActorClass()}, OnRequestItemActorClassComplete);
	}
}

void UItemObject::SpawnActorClass(const AActor* NewAnchor,
                                  const UClass* NewActorClass)
{
	if (!IsValid(NewActorClass))
	{
		return;
	}

	const bool bShouldSpawnAndAttach = HasRuntimeState(FGameplayTagContainer{UInventorySettings::GetEquippedTag()});

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const FTransform NewItemTransform = GetSpawningAnchorTransform(NewAnchor, bShouldSpawnAndAttach);
		RuntimeItemActor = World->SpawnActor(const_cast<UClass*>(NewActorClass), &NewItemTransform, FActorSpawnParameters());
		ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetInstancedTag()}, FGameplayTagContainer{UInventorySettings::GetPendingSpawnTag()});
	}

	if (bShouldSpawnAndAttach && ensureAlwaysMsgf(IsValid(RuntimeItemActor),
	                                              TEXT("Item Actor Class Failed to create an instance in World!")))
	{
		// @gdemers bad practice but avoid code refactoring!
		RuntimeItemActor->AttachToActor(const_cast<AActor*>(NewAnchor), FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	}
}

void UItemObject::OnSoftObjectAcquired(FOnRequestItemActorClassComplete Callback)
{
	if (!ItemProgressionStageHandle.IsValid())
	{
		Callback.ExecuteIfBound(nullptr, this);
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	ItemProgressionStageHandle->GetLoadedAssets(OutStreamableAssets);

	if (!OutStreamableAssets.IsEmpty())
	{
		Callback.ExecuteIfBound(Cast<UClass>(OutStreamableAssets[0]), this);
	}
	else
	{
		Callback.ExecuteIfBound(nullptr, this);
	}
}

void UItemObject::OnProgressionStageAcquired(FOnRequestItemActorClassComplete Callback)
{
	if (!ItemProgressionStageHandle.IsValid())
	{
		Callback.ExecuteIfBound(nullptr, this);
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	ItemProgressionStageHandle->GetLoadedAssets(OutStreamableAssets);

	if (OutStreamableAssets.IsEmpty())
	{
		Callback.ExecuteIfBound(nullptr, this);
		return;
	}

	const auto* ProgressionStageDefinitionDataAsset = Cast<UItemProgressionStageDefinitionDataAsset>(OutStreamableAssets[0]);
	if (IsValid(ProgressionStageDefinitionDataAsset))
	{
		FStreamableDelegate OnRequestItemActorClassComplete;
		OnRequestItemActorClassComplete.BindUObject(this, &UItemObject::OnSoftObjectAcquired, Callback);
		ItemProgressionStageHandle = UAssetManager::Get().LoadAssetList({ProgressionStageDefinitionDataAsset->GetOverrideItemActorClass()}, OnRequestItemActorClassComplete);
	}
	else
	{
		Callback.ExecuteIfBound(nullptr, this);
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
