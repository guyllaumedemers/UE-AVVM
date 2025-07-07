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
#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"

#include "ActorInventoryComponent.generated.h"

class UAVVMResourceManagerComponent;
class UItemObject;

/**
 *	Class description:
 *
 *	UInventoryLayoutHandler is the implementation behaviour that define how the UActorInventoryComponent
 *	content should be displayed.
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYSAMPLE_API UInventoryLayoutHandler : public UObject
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *	
 *	UActorInventoryComponent is the CORE component of the inventory system and allow ANY AActor to reference a set
 *	of items.
 *
 *	This system handle the loading request of all referenced items through the owning A UINTERFACE() IInventoryProvider api
 *	and provide layout information specific to the instance type at hand.
 *
 *	Note : This system participate in the 'Handshake' process following user interaction with UI and Ability execution for buying,
 *	selling, trading or consuming content.
 */
UCLASS(ClassGroup=("Inventory"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSAMPLE_API UActorInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DELEGATE(FOnAsyncSpawnRequestDeferred);

public:
	UActorInventoryComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void RequestItems(const UObject* Outer);

	UFUNCTION(BlueprintCallable)
	void SetupItems(const TArray<UObject*>& Resources);

	UFUNCTION(BlueprintCallable)
	void SetupItemProgressions(const TArray<UObject*>& Resources);

	UFUNCTION(BlueprintCallable)
	const TArray<UItemObject*>& GetItems() const;

	UFUNCTION(BlueprintCallable)
	const TArray<UItemObject*>& GetItemsByPartialMatch(const FGameplayTagContainer& FilteringTags) const;

	UFUNCTION(BlueprintCallable)
	const TArray<UItemObject*>& GetItemsByExactMatch(const FGameplayTagContainer& FilteringTags) const;

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags);

	UFUNCTION(BlueprintCallable)
	bool HasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool HasExactMatch(const FGameplayTagContainer& Compare) const;

protected:
	UFUNCTION()
	void OnItemsRetrieved();

	UFUNCTION()
	void OnRep_ItemCollectionChanged(const TArray<UItemObject*>& OldItemObjects);

	void SpawnEquipItem(UAVVMResourceManagerComponent* ResourceManagerComponent,
	                    UItemObject* NewItem);

	UFUNCTION()
	void OnItemActorClassRetrieved(const TSoftClassPtr<AActor>& NewActorClass,
	                               UItemObject* NewItemObject);

	struct FItemSpawnerQueuingMechanism
	{
		~FItemSpawnerQueuingMechanism();
		bool PushDeferredItem(UItemObject* NewItem, const UActorInventoryComponent::FOnAsyncSpawnRequestDeferred& NewRequest);
		bool TryExecuteNextRequest(const bool bCanDequeueFrontItem = false);
		bool HasPendingRequest() const;
		UItemObject* PopItem();

		TArray<UActorInventoryComponent::FOnAsyncSpawnRequestDeferred> PendingSpawnRequests;
		TArray<TWeakObjectPtr<UItemObject>> QueuedItems;
	};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UInventoryLayoutHandler> LayoutHandler = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_ItemCollectionChanged")
	TArray<TObjectPtr<UItemObject>> Items;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, meta=(ToolTip="GameplayTagContainer that define the state of the Outer Actor. Example : InTutorial, Pre-BossFight-X, etc..."))
	FGameplayTagContainer OwnedGameplayTags = FGameplayTagContainer::EmptyContainer;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
	FItemSpawnerQueuingMechanism QueuingMechanism;
};
