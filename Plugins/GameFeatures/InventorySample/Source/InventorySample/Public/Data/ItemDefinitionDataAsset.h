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
 *	UItemDefinitionDataAsset represent a singular item. It's retrieved based on a collection of items
 *	hosted from a backend services.
 *
 *	Example : Your microservice for the player account hold a collection of items tied to the player.
 *	We want to load them using a collection of FDataRegistryId and populate the relevant system Post-GameState::MatchStartup.
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYSAMPLE_API UItemDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemObject> ItemObject = nullptr;
};

/**
 *	Class description:
 *
 *	FAVVMItemDefinitionDataTableRow represent an entry in a DataTableRow for a unique item.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FAVVMItemDefinitionDataTableRow : public FAVVMDataTableRow
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
 *	UItemCollectionDefinitionDataAsset represent a STATIC collection of items to be referenced DIRECTLY by an Actor. This
 *	is for cases where an Actor should be interacted with and display non-dynamic content.
 *
 *	Example : A shop actor with a pre-defined set of items to buy from. 
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYSAMPLE_API UItemCollectionDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	const TArray<FDataRegistryId>& GetResourcesIds() const { return ItemRegistryIds; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDataRegistryId> ItemRegistryIds;
};

/**
 *	Class description:
 *
 *	FAVVMItemCollectionDefinitionDataTableRow represent an entry in a DataTableRow for a unique set of items.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FAVVMItemCollectionDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemCollectionDefinitionDataAsset> ItemCollectionDefinition = nullptr;
};
