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

void UItemObject::TrySpawnEquippedItem(const AActor* Target)
{
	auto* ResourceManagerComponent = IsValid(Target) ? Target->GetComponentByClass<UAVVMResourceManagerComponent>() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(ResourceManagerComponent), TEXT("Outer Actor doesn't reference a UAVVMResourceManagerComponent!")))
	{
		return;
	}

	OwningOuter = Target;

	FOnResourceAsyncLoadingComplete Callback;
	Callback.BindDynamic(this, &UItemObject::OnResourcesLoaded);
	ModifyRuntimeState(UInventorySettings::GetPendingSpawnEquipTags(), {});
	ResourceManagerComponent->RequestAsyncLoading(ItemProgressionId, Callback);
}

void UItemObject::TrySpawnDroppedItem(const AActor* Target)
{
	auto* ResourceManagerComponent = IsValid(Target) ? Target->GetComponentByClass<UAVVMResourceManagerComponent>() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(ResourceManagerComponent), TEXT("Outer Actor doesn't reference a UAVVMResourceManagerComponent!")))
	{
		return;
	}

	OwningOuter = Target;

	FOnResourceAsyncLoadingComplete Callback;
	Callback.BindDynamic(this, &UItemObject::OnResourcesLoaded);
	ModifyRuntimeState(UInventorySettings::GetPendingSpawnDropTags(), {});
	ResourceManagerComponent->RequestAsyncLoading(ItemProgressionId, Callback);
}

void UItemObject::OnResourcesLoaded()
{
	// TODO @gdemers The Resource Provider interface implementer Actor should handle the Spawining of the Actor in the Check function to be overriden.
	// The Data Asset at this point is already loaded and resources are forwarded to the check function. The only thing to do here would be to update the state
	// of the ItemObject!

	auto RemovedTags = FGameplayTagContainer::EmptyContainer;
	auto AddedTags = FGameplayTagContainer::EmptyContainer;

	const bool bHasPendingEquippedTags = HasRuntimeState(UInventorySettings::GetPendingSpawnEquipTags());
	if (bHasPendingEquippedTags)
	{
		RemovedTags = UInventorySettings::GetPendingSpawnEquipTags();
		AddedTags = UInventorySettings::GetInstancedEquippedTags();

		ModifyRuntimeState(AddedTags, RemovedTags);
		return;
	}

	const bool bHasPendingDroppedTags = HasRuntimeState(UInventorySettings::GetPendingSpawnDropTags());
	if (bHasPendingDroppedTags)
	{
		RemovedTags = UInventorySettings::GetPendingSpawnDropTags();
		AddedTags = UInventorySettings::GetInstancedDroppedTags();

		ModifyRuntimeState(AddedTags, RemovedTags);
		return;
	}

	// TODO @gdemers Impl in the Actor IResourceProvider::Check function 
	// const AActor* Outer = OwningOuter.Get();
	// if (!IsValid(Outer))
	// {
	// 	return;
	// }
	//
	// UWorld* World = GetWorld();
	// if (IsValid(World))
	// {
	// 	const FTransform ItemAnchorTransform = GetSpawningAnchorTransform(*Outer);
	// 	RuntimeItemActor = World->SpawnActor(nullptr, &ItemAnchorTransform, FActorSpawnParameters());
	// }
	//
	// if (ensureAlwaysMsgf(IsValid(RuntimeItemActor),
	//                      TEXT("Item Actor Class Failed to create an instance in World!")))
	// {
	// 	// @gdemers bad practice but avoid code refactoring!
	// 	RuntimeItemActor->AttachToActor(const_cast<AActor*>(Outer), FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	// }
}

FTransform UItemObject::GetSpawningAnchorTransform(const AActor& Target) const
{
	const bool bShouldSpawnAndAttach = HasRuntimeState(UInventorySettings::GetPendingSpawnEquipTags());
	const bool bShouldSpawnAndDrop = HasRuntimeState(UInventorySettings::GetPendingSpawnDropTags());

	FTransform ItemAnchorTransform = Target.GetActorTransform();
	if (bShouldSpawnAndAttach)
	{
		const auto* MeshComponent = Target.GetComponentByClass<UMeshComponent>();
		if (IsValid(MeshComponent))
		{
			ItemAnchorTransform = MeshComponent->GetSocketTransform(SocketName);
		}
	}
	else if (bShouldSpawnAndDrop)
	{
		// TODO @gdemers retrieve the item size and move by this increment
		const FVector3d NewLocation = ItemAnchorTransform.GetLocation() + (FVector3d::ForwardVector * 10.f);
		ItemAnchorTransform.SetLocation(NewLocation);
	}

	return ItemAnchorTransform;
}
