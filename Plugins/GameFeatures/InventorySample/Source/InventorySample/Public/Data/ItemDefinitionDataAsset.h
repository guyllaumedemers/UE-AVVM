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

#include "DataRegistryId.h"
#include "GameplayTagContainer.h"
#include "Data/AVVMDataTableRow.h"
#include "Engine/DataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "ItemDefinitionDataAsset.generated.h"

class UItemObject;

/**
 *	Class description:
 *	
 *	UItemDefinitionDataAsset is a singular item POD. It may be request for loading from various sources.
 *
 *		Example :
 *
 *		* based on a microservice request to load player account inventory.
 *		* based on a data asset referenced on a world actor.
 *
 *	This POD references the BP Class that will be instanced during gameplay.
 *	See UItemObject for details!
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYSAMPLE_API UItemDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	const TSoftClassPtr<UItemObject>& GetItemObjectClass() const;

	UFUNCTION(BlueprintCallable)
	bool CanAccessItem(const FGameplayTagContainer& RequirementTags,
	                   const FGameplayTagContainer& BlockingTags) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UItemObject> ItemObjectClass = nullptr;

	// @gdemers tags that define if this item can be accessed by the actor type.
	// Example : PlayerClass.Mage -> Cannot hold axe, but can hold staff
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RequiredTagsAllowingItemAccess = FGameplayTagContainer::EmptyContainer;

	// @gdemers tags that define if this item should not be accessed.
	// Example : Tag.InTutorial
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer BlockingTagsPreventingItemAccess = FGameplayTagContainer::EmptyContainer;
};

/**
 *	Class description:
 *
 *	FItemDefinitionDataTableRow is an entry in a DataTableRow for a unique UItemDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemDefinitionDataAsset> ItemDefinition = nullptr;
};

/**
 *	Class description:
 *	
 *	UItemGroupDefinitionDataAsset is a grouping of items to be referenced by an gameplay Actor class via the override of IResourceProvider.
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYSAMPLE_API UItemGroupDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	const TArray<FDataRegistryId>& GetItemIds() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDataRegistryId> ItemIds;
};

/**
 *	Class description:
 *
 *	FItemGroupDefinitionDataTableRow is an entry in a DataTableRow for a unique UItemGroupDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemGroupDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemGroupDefinitionDataAsset> ItemGroupDefinition = nullptr;
};
