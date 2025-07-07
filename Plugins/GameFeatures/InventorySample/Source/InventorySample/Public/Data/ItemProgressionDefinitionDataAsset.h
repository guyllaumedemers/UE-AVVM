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

#include "GameFramework/Actor.h"
#include "Data/AVVMDataTableRow.h"
#include "Engine/DataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "ItemProgressionDefinitionDataAsset.generated.h"

/**
 *	Class description:
 *
 *	UItemProgressionStageDefinitionDataAsset is a POD that reference information about an item representation at a given stage/level.
 *	This data asset can reference any information specific to your items and the overrides per-stages.
 *
 *	Example :
 *
 *		* Level 1: Skill Item A has Material A
 *		* Level 2: Skill Item A has Material B
 *		* etc...
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UItemProgressionStageDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	const FSoftObjectPath& GetOverrideItemActorClass() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	bool bDoesOverrideItemActorClass = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bDoesOverrideItemActorClass"))
	TSoftClassPtr<AActor> OverrideItemActorClass = nullptr;
};

/**
 *	Class description:
 *	
 *	UItemProgressionDefinitionDataAsset is a singular item and it's progression information.
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYSAMPLE_API UItemProgressionDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	const FSoftObjectPath& GetDefaultItemActorClass() const;

	UFUNCTION(BlueprintCallable)
	FSoftObjectPath GetProgressionStageItemActorOverride(const int32 ProgressionStageIndex) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftObjectPtr<const UItemProgressionStageDefinitionDataAsset>> ItemProgressionStageDataAssets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<AActor> DefaultItemActorClass = nullptr;
};

/**
 *	Class description:
 *
 *	FItemProgressionDefinitionDataTableRow is an entry in a DataTableRow for a unique item progression.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemProgressionDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemProgressionDefinitionDataAsset> ItemProgressionDefinition = nullptr;
};
