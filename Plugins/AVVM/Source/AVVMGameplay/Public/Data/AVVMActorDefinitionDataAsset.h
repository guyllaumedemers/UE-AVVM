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

#include "AVVMDataTableRow.h"
#include "DataRegistryId.h"
#include "Engine/DataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AVVMActorDefinitionDataAsset.generated.h"

/**
 *	Class description:
 *
 *	UAVVMActorDefinitionDataAsset is the unique definition of an Actor and the content that's expected to be in accessed at Runtime.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMActorDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	const FSoftObjectPath& GetActorSoftObjectPath() const;

	UFUNCTION(BlueprintCallable)
	TArray<FDataRegistryId> GetActorTraitIds() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(InlineEditConditionToggle))
	bool bDoesSupportActorOverride = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesSupportActorOverride"))
	TSoftClassPtr<AActor> OverrideActorClass = nullptr;

	// TODO BEGIN @gdemers Refactor this -- I no longer believe it fits with how the inventory system handle progression. MOve this into a ActorTraitsDefinition
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(InlineEditConditionToggle))
	bool bDoesSupportPassiveAbilities = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesSupportPassiveAbilities", ItemStruct="AVVMAbilityGroupDefinitionDataTableRow"))
	TArray<FDataRegistryId> PassiveAbilityGroupIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(InlineEditConditionToggle))
	bool bDoesSupportActiveAbilities = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesSupportActiveAbilities", ItemStruct="AVVMAbilityGroupDefinitionDataTableRow"))
	TArray<FDataRegistryId> ActiveAbilityGroupIds;
	// TODO END
};

/**
 *	Class description:
 *
 *	FAVVMActorDefinitionDataTableRow is an entry in a DataTableRow for a unique Actor type.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UAVVMActorDefinitionDataAsset> ActorDefinition = nullptr;
};
