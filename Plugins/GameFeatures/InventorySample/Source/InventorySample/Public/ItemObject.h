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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "DataRegistryId.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "ItemObject.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnRequestItemActorClassComplete, const UClass*, NewActorClass, class UItemObject*, NewItemObject);

/**
 *	Class description:
 *
 *	FItemState is the state of the Item portrait using Tags.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemState
{
	GENERATED_BODY()

	// @gdemers Complex state. Example : CanBeConsumed & InBackupStorage & PendingForTrade
	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTagContainer StateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 Counter = 1;
};

/**
 *	Class description:
 *	
 *	UItemObject is any object that exist in your project LORE. It may be loaded via a FDataRegistryId (targeting: UItemDefinitionDataAsset),
 *	instanced, and tracks the state of the object based on user actions.
 *
 *		Example :
 *
 *			* ItemState::InPlayerInventory->InGameWorld will spawn an Actor defined based on this object configuration for pickup.
 *			* ItemState::InGameWorld may also imply we are previewing an item for purchased from a Shop.
 *			* etc...
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UItemObject : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags);

	UFUNCTION(BlueprintCallable)
	bool HasRuntimeState(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool HasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool HasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	const FDataRegistryId& GetItemProgressionId() const;

	UFUNCTION(BlueprintCallable)
	void GetItemActorClassAsync(const UObject* NewProgressionDefinitionData,
	                            const int32 NewProgressionStageIndex,
	                            const FOnRequestItemActorClassComplete& OnRequestItemActorClassComplete);

	UFUNCTION(BlueprintCallable)
	void SpawnActorClass(const AActor* NewAnchor,
	                     const UClass* NewActorClass);

protected:
	UFUNCTION()
	void OnSoftObjectAcquired(FOnRequestItemActorClassComplete Callback);

	UFUNCTION()
	void OnProgressionStageAcquired(FOnRequestItemActorClassComplete Callback);

	FTransform GetSpawningAnchorTransform(const AActor* NewOuter, const bool bShouldAttachToSocket) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="Define the Item State and how it should behave. Example : CanBeConsumed, Destroy on Drop, etc..."))
	FGameplayTagContainer DefaultItemStateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="Define the Item Category. Example : Weapon, Armor, etc... and the Slot where it can be equipped!"))
	FGameplayTagContainer ItemTypeTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDataRegistryId ItemProgressionId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	bool bCanAttachToSocket = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bCanAttachToSocket"))
	FName SocketName = NAME_None;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AActor> RuntimeItemActor = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FItemState RuntimeItemState = FItemState();

	TSharedPtr<FStreamableHandle> ItemProgressionStageHandle;
};
