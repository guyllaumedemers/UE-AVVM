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
#include "Engine/DeveloperSettings.h"

#include "InventorySettings.generated.h"

/**
 *	Class description:
 *
 *	UInventorySettings is a DeveloperSettings that expose global tags use to track/update the state of an Item.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UInventorySettings"))
class INVENTORYSAMPLE_API UInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTag& GetPendingSpawnTag();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTag& GetInstancedTag();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTag& GetEquippedTag();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetEmptyItemCount_BlockedActions();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetFullInventory_BlockedActions();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FName GetItemMaxStackCount(const FGameplayTag& StackCategoryTag);

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const TSoftObjectPtr<UDataTable>& GetItemMaxStackCountDataTable();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetStorageRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetHoldingRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetEquippedRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetPassiveRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetOffensiveRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetDefensiveRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetConsumableRuleset();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FString& GetAppDataDirPath();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const TSoftObjectPtr<UDataTable>& GetDefaultProviderInventories();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTag& GetStorageTypeTag();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTag& GetAttachmentTypeTag();

	UFUNCTION(BlueprintCallable, Category="Inventory|Settings")
	static const FGameplayTagContainer& GetItemActorSpawnConditions();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTag PendingSpawnTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTag InstancedTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTag EquippedTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer EmptyItemCount_BlockedActions = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer FullInventory_BlockedActions = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TMap<FGameplayTag/*Item Category*/, FName/*RowName*/> ItemMaxStackCounts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSoftObjectPtr<UDataTable> ItemMaxStackCountDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer StorageRuleset = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer HoldingRuleset = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer EquippedRuleset = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer PassiveRuleset = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer OffensiveRuleset = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer DefensiveRuleset = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer ConsumableRuleset = FGameplayTagContainer::EmptyContainer;

	// @gdemers Path to serialize inventory layout of Actors who's content require progression tracking.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FString AppDataDirPath = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSoftObjectPtr<UDataTable> DefaultProviderInventories;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTag StorageTypeTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTag AttachmentTypeTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer ItemActorSpawnConditions = FGameplayTagContainer::EmptyContainer;
};
