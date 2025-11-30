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
#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "ItemObject.generated.h"

struct FStreamableHandle;
class UAttributeSet;
class UItemObject;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnRequestItemActorClassComplete, const UClass*, NewActorClass, const FSoftObjectPath&, NewActorAttributeSetSoftObjectPath, UItemObject*, NewItemObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRuntimeStateChanged, const FGameplayTagContainer&, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRuntimeCountChanged, const int32, NewState);

/**
 *	Class description:
 *
 *	FItemState is the state of the Item portrait using Tags.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemState
{
	GENERATED_BODY()

	// @gdemers Complex state. Example : CanBeConsumed & IsAccessible & PendingForTrade
	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTagContainer StateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 Counter = 1;
};

/**
 *	Class description:
 *	
 *	FItemActorSpawnContextArgs is a context struct that encapsulate information required for Spawning an Actor class
 *	in the inventory system.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemActorSpawnContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	const UClass* ActorClass = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	FSoftObjectPath AttributeSetSoftObjectPath = FSoftObjectPath();

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<AActor> Outer = nullptr;
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
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
	                                          UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags);

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeCount(const int32 NewCountModifier);

	UFUNCTION(BlueprintCallable)
	bool DoesRuntimeStateEquals(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesTypeHasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesTypeHasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesBehaviourHasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesBehaviourHasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetRuntimeState() const;

	UFUNCTION(BlueprintCallable)
	bool DoesRuntimeStateHasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesRuntimeStateHasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable)
	int32 GetRuntimeCount() const;

	UFUNCTION(BlueprintCallable)
	const FDataRegistryId& GetItemActorId() const;

	UFUNCTION(BlueprintCallable)
	const FDataRegistryId& GetItemProgressionId() const;

	UFUNCTION(BlueprintCallable)
	void GetItemActorClassAsync(const UObject* NewActorDefinitionDataAsset,
	                            const FOnRequestItemActorClassComplete& OnRequestItemActorClassComplete);

	UFUNCTION(BlueprintCallable)
	AActor* SpawnActor(const FItemActorSpawnContextArgs& ContextArgs);

	UPROPERTY(BlueprintAssignable)
	FOnItemRuntimeStateChanged OnItemRuntimeStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnItemRuntimeCountChanged OnItemRuntimeCountChanged;

protected:
	UFUNCTION()
	void OnItemActorClassAcquired(FOnRequestItemActorClassComplete Callback, const FSoftObjectPath NewActorAttributeSetSoftObjectPath);

	UFUNCTION()
	void OnRep_ItemStateModified(const FItemState& OldItemState);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Item behaviour. Example : Destroy on Drop, Cannot be trade, NPC owned, etc..."))
	FGameplayTagContainer ItemBehaviourTypeTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Item Category. Example : Passive, Offensive, Defensive, Consumable, etc... Allow building complex types."))
	FGameplayTagContainer ItemTypeTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Slot Tags in which the item can be slotted in the loadout system."))
	FGameplayTagContainer ItemSlotTags = FGameplayTagContainer::EmptyContainer;

	// @gdemers Item progression is referenced at this level to prevent hard references on Actor type defined in other GFP and
	// applies progression effects to the loaded actor type.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="ItemProgressionDefinitionDataTableRow"))
	FDataRegistryId ItemProgressionId = FDataRegistryId();

	// @gdemers UItemObject is not an Actor type as we wouldn't be able to derive from it in other GFP.
	// Using RegistryId, we are working around boundaries constraint created by GFP dlls.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorDefinitionDataTableRow"))
	FDataRegistryId ItemActorId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FName SocketName = NAME_None;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AActor> RuntimeItemActor = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_ItemStateModified")
	FItemState RuntimeItemState = FItemState();

	TSharedPtr<FStreamableHandle> ItemActorHandle;
};
