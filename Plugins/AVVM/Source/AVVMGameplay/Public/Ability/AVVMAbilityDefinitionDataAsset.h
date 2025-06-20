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

#include "AVVMAbilityDefinitionDataAsset.generated.h"

class UGameplayAbility;

/**
 *	Class description:
 *
 *	UAVVMAbilityDefinitionDataAsset represent a single ability and it's requirements for being granted.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMAbilityDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	bool CanGrantAbility(const FGameplayTagContainer& RequirementTags,
	                     const FGameplayTagContainer& BlockingTags) const;

	UFUNCTION(BlueprintCallable)
	const TSoftClassPtr<UGameplayAbility>& GetGameplayAbilityClass() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UGameplayAbility> GameplayAbilityClass = nullptr;

	// @gdemers tags that define if this ability can be granted to the actor type.
	// Example : Tag.IsPlayer, Tag.IsFlyingType
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RequiredTagsForGrantingAbility = FGameplayTagContainer::EmptyContainer;

	// @gdemers tags that define if this ability should not be granted.
	// Example : Tag.IsWorldWaterLevel -> Blocks Tag.IsFlyingType
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer BlockingTagsPreventingGrantingAbility = FGameplayTagContainer::EmptyContainer;
};

/**
 *	Class description:
 *
 *	FAVVMAbilityDefinitionDataTableRow is an entry in a DataTableRow for a unique UAVVMAbilityDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMAbilityDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMAbilityDefinitionDataAsset> AbilityDefinitionDataAsset = nullptr;
};

/**
 *	Class description:
 *
 *	UAVVMAbilityGroupDefinitionDataAsset represent a set of abilities to be granted to an Actor with an ASC.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMAbilityGroupDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	const TArray<FDataRegistryId>& GetAbilityIds() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDataRegistryId> AbilityIds;
};

/**
 *	Class description:
 *
 *	FAVVMAbilityGroupDefinitionDataTableRow is an entry in a DataTableRow for a unique UAVVMAbilityGroupDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMAbilityGroupDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMAbilityGroupDefinitionDataAsset> AbilityGroupDefinitionDataAsset;
};
