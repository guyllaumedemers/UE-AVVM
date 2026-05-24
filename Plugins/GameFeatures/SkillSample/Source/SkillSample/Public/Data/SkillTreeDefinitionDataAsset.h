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

#include "SkillTreeDefinitionDataAsset.generated.h"

class USkillTreeNodeObject;

/**
 *	Class description:
 *	
 *	USkillTreeNodeDefinitionDataAsset is a singular Node POD. It may be request for loading from various sources.
 *
 *		Example :
 *
 *		* based on a microservice request to load player account inventory.
 *		* based on a data asset referenced on a world actor.
 *
 *	This POD references the BP Class that will be instanced during gameplay.
 *	See USkillTreeNodeObject for details!
 */
UCLASS(BlueprintType, NotBlueprintable)
class SKILLSAMPLE_API USkillTreeNodeDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	const TSoftClassPtr<USkillTreeNodeObject>& GetSkillTreeNodeObjectClass() const;

	bool CanAccessSkillTreeNodeObject(const FGameplayTagContainer& RequirementTags,
	                                  const FGameplayTagContainer& BlockingTags) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<USkillTreeNodeObject> SkillTreeNodeObjectClass = nullptr;

	// @gdemers tags that define if this entity can be accessed by the actor type.
	// Example : PlayerClass.Warrior -> Cannot use non-warrior abilities.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer RequiredTagsForAccess = FGameplayTagContainer::EmptyContainer;

	// @gdemers tags that define if this entity should not be accessed.
	// Example : Tag.InTutorial
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer BlockingTagsForAccess = FGameplayTagContainer::EmptyContainer;
};

/**
 *	Class description:
 *
 *	FSkillTreeNodeObjectDefinitionDataTableRow is an entry in a DataTableRow for a unique USkillTreeNodeDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeNodeObjectDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<USkillTreeNodeDefinitionDataAsset> SkillTreeObjectDefinition = nullptr;
};

/**
 *	Class description:
 *	
 *	USkillTreeDefinitionDataAsset is a grouping of Nodes, and reference the entity that component the Actor Skill Tree.
 */
UCLASS(BlueprintType, NotBlueprintable)
class SKILLSAMPLE_API USkillTreeDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	const TArray<FDataRegistryId>& GetSkillTreeNodeObjectIds() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="SkillTreeNodeObjectDefinitionDataTableRow"))
	TArray<FDataRegistryId> SkillTreeNodeIds;
};

/**
 *	Class description:
 *
 *	FSkillTreeDefinitionDataTableRow is an entry in a DataTableRow for a unique USkillTreeDefinitionDataAsset. It represents the
 *	logical construct of an Actor Skill Tree.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<USkillTreeDefinitionDataAsset> SkillTreeDefinition = nullptr;
};
