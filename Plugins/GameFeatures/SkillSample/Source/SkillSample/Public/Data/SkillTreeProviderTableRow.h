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
#include "Engine/DataTable.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "SkillTreeProviderTableRow.generated.h"

/**
 *	Class description:
 *	
 *	FSkillTreeNodePhase define the Tree Node GameplayEffects an actor owns during a given phase.
 *	
 *	Note : Here we expect designers to manage properly Tree Node referencing, and respect class specific
 *	skills. Example : Don't reference a Mage skill onto a warrior, unless your project support sub-classing.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeNodePhase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="SkillTreeNodeObjectDefinitionDataTableRow"))
	TMap<FDataRegistryId, int32 /*GameplayEffect level*/> SkillTreeNodeIds;
};

/**
 *	Class description:
 *	
 *	FSkillTreeProviderTableRow is a Row type that define the default values used to initialize an
 *	SkillTree Provider entry on disk.
 *	
 *	Note : This is used to default initialize an actor representation Skill Tree based on design configuration.
 *	Example : A boss in elden ring.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeProviderTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// @gdemers the unique identifier that represent the skill tree provider actor. example : a shop.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorIdentifierDataTableRow"))
	FDataRegistryId SkillTreeProviderActorIdentifierId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<FSkillTreeNodePhase> SkillTreeNodePerPhases;
};
