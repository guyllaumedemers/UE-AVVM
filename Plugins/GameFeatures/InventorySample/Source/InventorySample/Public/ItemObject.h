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
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "ItemObject.generated.h"

struct FStreamableHandle;
class UAttributeSet;
class UDataTable;
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

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 StorageId = NULL;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 StoragePosition = NULL;
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
	void ModifyRuntimeStorageId(const int32 NewStorageId);

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeStoragePosition(const int32 NewStoragePosition);

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
	bool DoesAttachmentSlotHasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesAttachmentSlotHasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesSlotHasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesSlotHasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetRuntimeState() const;

	UFUNCTION(BlueprintCallable)
	bool DoesRuntimeStateHasPartialMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool DoesRuntimeStateHasExactMatch(const FGameplayTagContainer& Compare) const;

	UFUNCTION(BlueprintCallable)
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable)
	bool CanStack(const UItemObject* Item) const;

	UFUNCTION(BlueprintCallable)
	bool Stack(UItemObject* Item);

	UFUNCTION(BlueprintCallable)
	int32 GetMaxStackCount() const;

	UFUNCTION(BlueprintCallable)
	int32 GetRuntimeCount() const;

	const FDataRegistryId& GetItemActorId() const;

	void GetItemActorClassAsync(const UObject* NewActorDefinitionDataAsset,
	                            const FOnRequestItemActorClassComplete& OnRequestItemActorClassComplete);

	void SpawnActor(const FItemActorSpawnContextArgs& ContextArgs);

	// @gdemers read data specific to all attachments equipped on the given UItemObject referenced actor.
	const TMap<FGameplayTag, TWeakObjectPtr<const AActor>>& GetNonReplicatedItemAttachmentActors() const;

	UPROPERTY(BlueprintAssignable)
	FOnItemRuntimeStateChanged OnItemRuntimeStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnItemRuntimeCountChanged OnItemRuntimeCountChanged;

protected:
	UFUNCTION()
	void OnItemActorClassAcquired(FOnRequestItemActorClassComplete Callback, const FSoftObjectPath NewActorAttributeSetSoftObjectPath);

	UFUNCTION()
	void OnRep_ItemStateModified(const FItemState& OldItemState);

	UFUNCTION()
	void OnNewSocketItemAttached(const FGameplayTag& NewItemAttachmentSlotTag,
	                             const AActor* NewAttachment);

	UFUNCTION()
	void OnNewSocketItemDetached(const FGameplayTag& NewItemAttachmentSlotTag);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Item behaviour. Example : Destroy on Drop, Cannot be trade, NPC owned, etc..."))
	FGameplayTagContainer ItemBehaviourTypeTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Item Category. Example : Passive, Offensive, Defensive, Consumable, etc... Allow building complex types."))
	FGameplayTagContainer ItemTypeTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Slot Tags in which the item can be slotted in the loadout system."))
	FGameplayTagContainer ItemSlotTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Define the Slot Tags that compose this item. Usefull when attaching Modifiers at Runtime."))
	FGameplayTagContainer ItemAttachmentSlotTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Points to a global mapping of tag to int32."))
	FGameplayTag MaxStackCount_CategoryTag = FGameplayTag::EmptyTag;

	// @gdemers UItemObject is not an Actor type as we wouldn't be able to derive from it in other GFP.
	// Using RegistryId, we are working around boundaries constraint created by GFP dlls.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorDefinitionDataTableRow"))
	FDataRegistryId ItemActorId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FName SocketName = NAME_None;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AActor> RuntimeItemActor = nullptr;

	// @gdemers server-sided property that cache information about runtime attachments bounds to the actor
	// referenced by this UItemObject instance. This imply that its not available during gameplay on the client-side which may
	// not be what you want for your game, but still make its possible to customize/display item configuration in offline scenarios (example : menu lobby).
	// Note : Alternatives can be put in place to forward that data via RPC following client-sided request.
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<const AActor>> NonReplicatedItemAttachmentActors;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_ItemStateModified")
	FItemState RuntimeItemState = FItemState();

	TSharedPtr<FStreamableHandle> ItemActorHandle = nullptr;
	FDelegateHandle OnNewSocketAttachedHandle;
	FDelegateHandle OnNewSocketDetachedHandle;

private:
	int32 PrivateItemId = INDEX_NONE;
	friend class UItemObjectUtils;
};

/**
 *	Class description:
 *	
 *	UItemObjectUtils is a blueprint function library that expose reusable api.
 */
UCLASS()
class INVENTORYSAMPLE_API UItemObjectUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static int32 RuntimeInit(const UObject* Outer,
	                         const TArray<int32>& NewPrivateIds,
	                         const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
	                         UItemObject* UnInitializedItemObject);

	UFUNCTION(BlueprintCallable)
	static int32 GetMaxStackCount(const UDataTable* MaxStackCountDataTable,
	                              const FGameplayTag& MaxStackCountTag);

	UFUNCTION(BlueprintCallable)
	static int32 GetItemStartupStackCount(const UItemObject* UnInitializedItemObject,
	                                      const int32 ElementId);

	UFUNCTION(BlueprintCallable)
	static int32 GetNumSplits(const UItemObject* SrcItem);

	UFUNCTION(BlueprintCallable)
	static UItemObject* SplitObject(UObject* Outer, UItemObject* SrcItem);

	UFUNCTION(BlueprintCallable)
	static AActor* SpawnWorldItemActor(const UWorld* World, UItemObject* SrcItem);

	UFUNCTION(BlueprintCallable)
	static void DestroyWorldItemActor(const UItemObject* SrcItem);
};
