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
 *	UAVVMActorDefinitionDataAsset define a set of properties to be loaded and applied to an Actor.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMActorDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesSupportPassiveAbilities = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bDoesSupportPassiveAbilities"))
	FDataRegistryId PassiveAbilityGroupId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesSupportActiveAbilities = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bDoesSupportActiveAbilities"))
	FDataRegistryId ActiveAbilityGroupId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesSupportCosmetic = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bDoesSupportCosmetic"))
	FDataRegistryId CosmeticDefinitionId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesSupportEquipment = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bDoesSupportEquipment"))
	FDataRegistryId EquipmentDefinitionId = FDataRegistryId();
};

/**
 *	Class description:
 *
 *	FAVVMActorDefinitionDataTableRow override the base list of resources to be dynamically loaded by the Resource Manager component
 *	for it's counter-part Actor type.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMActorDefinitionDataAsset> ActorDefinition = nullptr;
};
