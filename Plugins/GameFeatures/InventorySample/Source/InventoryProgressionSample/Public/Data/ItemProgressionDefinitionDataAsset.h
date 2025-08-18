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

#include "ItemProgressionDefinitionDataAsset.generated.h"

class UGameplayEffect;

/**
 *	Class description:
 *
 *	UItemProgressionStageDefinitionDataAsset is a POD that reference a collection of effects to be applied at a given level.
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYPROGRESSIONSAMPLE_API UItemProgressionStageDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	TArray<FSoftObjectPath> GetProgressionStackingEffectSoftObjectPaths() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<TSoftClassPtr<UGameplayEffect>/*Stacking GE*/> ProgressionGameplayEffects;
};

/**
 *	Class description:
 *	
 *	UItemProgressionDefinitionDataAsset is a singular item and it's progression information.
 */
UCLASS(BlueprintType, NotBlueprintable)
class INVENTORYPROGRESSIONSAMPLE_API UItemProgressionDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	TArray<FSoftObjectPath> GetProgressionStages(const int32 ProgressionStageIndex) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(InlineEditConditionToggle))
	bool bDoesApplyProgressionEffects = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesApplyProgressionEffects"))
	TMap<int32 /*Stage Level*/, TSoftObjectPtr<const UItemProgressionStageDefinitionDataAsset>> ItemProgressionStageDataAssets;
};

/**
 *	Class description:
 *
 *	FItemProgressionDefinitionDataTableRow is an entry in a DataTableRow for a unique item progression.
 */
USTRUCT(BlueprintType)
struct INVENTORYPROGRESSIONSAMPLE_API FItemProgressionDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UItemProgressionDefinitionDataAsset> ItemProgressionDefinition = nullptr;
};
