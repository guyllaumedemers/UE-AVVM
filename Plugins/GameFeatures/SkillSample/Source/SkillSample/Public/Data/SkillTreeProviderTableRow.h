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

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "SkillTreeProviderTableRow.generated.h"

/**
 *	Class description:
 *	
 *	FSkillTreeNodeData define the Tree Node GameplayEffect an actor owns.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeNodeDefinition
{
	GENERATED_BODY()

	static const int32 Static_GetRelationshipBitmask(const FSkillTreeNodeDefinition& SkillTreeNodePhase);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer RelationshipTags{FGameplayTagContainer::EmptyContainer};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="SkillTreeNodeObjectDefinitionDataTableRow"))
	FDataRegistryId SkillTreeNodeId{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="1", ClampMax="63"))
	int32 InstancedId{INDEX_NONE};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="1", ClampMax="63"))
	int32 EffectLevel{INDEX_NONE};
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
struct SKILLSAMPLE_API FSkillTreeProviderTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// @gdemers the unique identifier that represent the skill tree provider actor. example : a shop.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorIdentifierDataTableRow"))
	FDataRegistryId SkillTreeProviderActorIdentifierId{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<FSkillTreeNodeDefinition> SkillTreeNodeDefinitions{};
};
