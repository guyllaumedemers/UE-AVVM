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

#include "AVVMAbilityData.generated.h"

class UInputAction;
class UGameplayAbility;

/**
 *	Class description:
 *
 *	UAVVMAbilityDataAsset represent a single ability and it's requirements for being granted.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	bool CanGrantAbility(const FGameplayTagContainer& ContextualTags/*world specific*/) const;

	UFUNCTION(BlueprintCallable)
	const TSoftClassPtr<UGameplayAbility>& GetGameplayAbilityClass() const;

	UFUNCTION(BlueprintCallable)
	const TSoftObjectPtr<UInputAction>& GetInputAction() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UGameplayAbility> GameplayAbility = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	bool bIsPassiveAbility = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="!bIsPassiveAbility"))
	TSoftObjectPtr<UInputAction> AbilityInputAction = nullptr;

	// @gdemers tags that define if this ability can be granted to the actor type.
	// Example : Tag.IsPlayer, Tag.IsFlyingType
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RequiredTagsForGrantingAbility = FGameplayTagContainer::EmptyContainer;

	// @gdemers tags that define if this ability can block granting.
	// Example : Tag.IsWorldWaterLevel -> Blocks Tag.IsFlyingType
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer BlockingTagsPreventingGrantingAbility = FGameplayTagContainer::EmptyContainer;
};

/**
 *	Class description:
 *
 *	FAVVMAbilityDataTableRow represent a single ability entry in a Data Table.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMAbilityDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMAbilityDataAsset> AbilityDataAsset = nullptr;
};

/**
 *	Class description:
 *
 *	UAVVMAbilityGroupDataAsset represent a set of abilities to be granted to an Actor with an ASC.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMAbilityGroupDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	const TArray<FDataRegistryId>& GetAbilities() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDataRegistryId> Abilities;
};

/**
 *	Class description:
 *
 *	FAVVMAbilityGroupDataTableRow. Struct Class referenced in UDataTable and accessed via a Registry Id. It's expected
 *	to be gathered through UMetaDataRegistrySources_DataTable across multiple GFP at editor time.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMAbilityGroupDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UAVVMAbilityGroupDataAsset>> AbilityGroupDataAssets;
};
