﻿//Copyright(c) 2025 gdemers
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
class UNonReplicatedLoadoutObject;

/**
 *	Class description:
 *
 *	FItemToken describe a unique identifier that increment only when default construct. Can be safely
 *	passed by copy around.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemToken
{
	GENERATED_BODY()

	explicit FItemToken()
	{
		static uint32 GlobalUniqueId = 0;
		UniqueId = ++GlobalUniqueId;
	}

	UPROPERTY()
	uint32 UniqueId = 0;
};

/**
 *	Class description:
 *	
 *	UActorInventoryComponent is the CORE component of the inventory system and allow ANY AActor to reference a set
 *	of items.
 *
 *	This system handle the loading request of all referenced items through the owning Actor implementation of the IInventoryProvider api.
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
	static UActorInventoryComponent* GetActorComponent(const AActor* NewActor);

	UFUNCTION(BlueprintCallable)
	void RequestItems(const AActor* Outer);

	void SetupItems(const TArray<UObject*>& NewResources);
	void SetupItemActors(const TArray<UObject*>& NewResources);

	UFUNCTION(BlueprintCallable)
	const TArray<UItemObject*>& GetItems() const;

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags);

	UFUNCTION(BlueprintCallable)
	bool HasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool HasExactMatch(const FGameplayTagContainer& Compare) const;

protected:
	UFUNCTION()
	void OnItemsRetrieved(FItemToken ItemToken);

	UFUNCTION()
	void OnRep_ItemCollectionChanged(const TArray<UItemObject*>& OldItemObjects);

	void SpawnEquipItem(UAVVMResourceManagerComponent* ResourceManagerComponent,
	                    UItemObject* NewItem);

	UFUNCTION()
	void OnItemActorClassRetrieved(const UClass* NewActorClass, UItemObject* NewItemObject);
	
	UFUNCTION()
	void OnLoadoutObjectRetrieved();

	struct FItemSpawnerQueuingMechanism
	{
		~FItemSpawnerQueuingMechanism();
		bool PushDeferredItem(UItemObject* NewItem, const UActorInventoryComponent::FOnAsyncSpawnRequestDeferred& NewRequest);
		bool TryExecuteNextRequest(const bool bCanDequeueFrontItem = false);
		bool HasPendingRequest() const;
		UItemObject* PeekItem() const;

		TArray<UActorInventoryComponent::FOnAsyncSpawnRequestDeferred> PendingSpawnRequests;
		TArray<TWeakObjectPtr<UItemObject>> QueuedItems;
	};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<UNonReplicatedLoadoutObject> NonReplicatedLoadoutClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_ItemCollectionChanged")
	TArray<TObjectPtr<UItemObject>> Items;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, meta=(ToolTip="GameplayTagContainer that define the state of the Outer Actor. Example : InTutorial, Pre-BossFight-X, etc..."))
	FGameplayTagContainer ComponentStateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UNonReplicatedLoadoutObject> NonReplicatedLoadout = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TMap<uint32, TSharedPtr<FStreamableHandle>> ItemHandleSystem;
	TSharedPtr<FItemSpawnerQueuingMechanism> QueueingMechanism = nullptr;
	TSharedPtr<FStreamableHandle> LoadoutHandle = nullptr;
};
