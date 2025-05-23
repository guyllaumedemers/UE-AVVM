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
UCLASS(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	bool CanGrantAbility(const FGameplayTagContainer& ActorActiveTags/*actor specific*/,
	                     const FGameplayTagContainer& ContextualTags/*world specific*/) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UGameplayAbility> GameplayAbility = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
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
 *	UAVVMAbilityGroupDataAsset represent a set of abilities to be granted to an Actor with an ASC.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMAbilityGroupDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UAVVMAbilityDataAsset>> Abilities;
};

/**
 *	Class description:
 *
 *	FAVVMAbilityGroupDataTableRow. Struct Class referenced in UDataTable and accessed via a UDataRegistry. It's expected
 *	to be retrieved through UMetaDataRegistrySources_DataTable at Runtime.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMAbilityGroupDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMAbilityGroupDataAsset> AbilityGroupDataAsset = nullptr;
};
