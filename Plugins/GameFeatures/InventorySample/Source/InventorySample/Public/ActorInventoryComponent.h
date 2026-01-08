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

#include "ExecutionContextParams.h"
#include "ExecutionContextRule.h"
#include "GameplayTagContainer.h"
#include "Backend/AVVMDataResolverHelper.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"

#include "ActorInventoryComponent.generated.h"

struct FStreamableHandle;
class UAVVMResourceManagerComponent;
class UItemObject;
class UNonReplicatedLoadoutObject;
class UNonReplicatedWeightManagerObject;

/**
 *	Class description:
 *	
 *	FInventoryDataResolverHelper is a context struct that resolve backend information about an Actor inventory.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FInventoryDataResolverHelper : public FAVVMDataResolverHelper
{
	GENERATED_BODY()

	virtual TArray<int32> GetElementDependencies(const UObject* Outer, const int32 ElementId) const override;
};

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

	UFUNCTION(BlueprintCallable)
	const TArray<UItemObject*>& GetItems() const;

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags);

	UFUNCTION(BlueprintCallable)
	bool HasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool HasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool CheckWeightOverflow(const UItemObject* NewItemObject) const;

	UFUNCTION(BlueprintCallable)
	void Drop(UItemObject* PendingDropItemObject);

	UFUNCTION(BlueprintCallable)
	void Pickup(UItemObject* PendingPickupItemObject);

	UFUNCTION(BlueprintCallable)
	void Swap(UItemObject* SrcItemObject, UItemObject* DestItemObject);

protected:
	UFUNCTION()
	void OnItemsRetrieved(FItemToken ItemToken);

	UFUNCTION()
	void OnRep_ItemCollectionChanged(const TArray<UItemObject*>& OldItemObjects);

	void SpawnEquipItem(UAVVMResourceManagerComponent* ResourceManagerComponent,
	                    UItemObject* NewItem);

	UFUNCTION()
	void OnItemActorClassRetrieved(const UClass* NewActorClass,
	                               const FSoftObjectPath& NewActorAttributeSetSoftObjectPath,
	                               UItemObject* NewItemObject);
	
	UFUNCTION()
	void OnWeightManagerObjectRetrieved();
	
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

	bool CanExecute(const TInstancedStruct<FExecutionContextParams>& Params,
	                const TInstancedStruct<FExecutionContextRule>& Rule) const;
	
	UFUNCTION(Server, Reliable)
	void Server_Drop(UItemObject* PendingDropItemObject);
	
	UFUNCTION(Server, Reliable)
	void Server_Pickup(UItemObject* PendingDropItemObject);
	
	UFUNCTION(Server, Reliable)
	void Server_Swap(UItemObject* SrcItemObject, UItemObject* DestItemObject);

	void CheckBackend();
	void CheckBounds();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<UNonReplicatedWeightManagerObject> NonReplicatedWeightManagerClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<UNonReplicatedLoadoutObject> NonReplicatedLoadoutClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer OuterDropConditionTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_ItemCollectionChanged")
	TArray<TObjectPtr<UItemObject>> Items;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, meta=(ToolTip="GameplayTagContainer that define the state of the Outer Actor. Example : InTutorial, Pre-BossFight-X, etc..."))
	FGameplayTagContainer ComponentStateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UNonReplicatedWeightManagerObject> NonReplicatedWeightManager = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UNonReplicatedLoadoutObject> NonReplicatedLoadout = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TMap<uint32, TSharedPtr<FStreamableHandle>> ItemHandleSystem;
	TSharedPtr<FItemSpawnerQueuingMechanism> QueueingMechanism = nullptr;
	TSharedPtr<FStreamableHandle> WeightManagerHandle = nullptr;
	TSharedPtr<FStreamableHandle> LoadoutHandle = nullptr;

private:
	void SetupItemObjects(const TArray<UObject*>& NewResources);
	void SetupItemActors(const TArray<UObject*>& NewResources);
	
	// @gdemers Data Resolver for backend representation of an actor inventory. 
	static const TInstancedStruct<FAVVMDataResolverHelper>& GetInventoryDataResolverHelper();

	// @gdemers ai version of this component may require different rules, or any in-game static actor
	// such as destructables.
	virtual TInstancedStruct<FExecutionContextRule> GetDropRule() const;
	virtual TInstancedStruct<FExecutionContextRule> GetPickupRule() const;
	virtual TInstancedStruct<FExecutionContextRule> GetSwapRule() const;

	// @gdemers virtual overrides are available. respect property access modifiers.
	virtual void OnDrop(UItemObject* ItemObject);
	virtual void OnPickup(UItemObject* ItemObject);
	virtual void OnSwap(UItemObject* SrcItemObject, UItemObject* DestItemObject);
	
	UFUNCTION(meta=(AllowPrivateAccess))
	void OnOuterTagChanged(const FGameplayTagContainer& NewTags);

	// @gdemers cached representation of what has been attributed during the initialization
	// phase of our inventory. This address the problem of uniqueness for entries with identical type.
	TArray<int32> PrivateItemIds;
	
	friend class UInventoryResourceHandlingImpl;
	friend class UItemObjectUtils;
};
