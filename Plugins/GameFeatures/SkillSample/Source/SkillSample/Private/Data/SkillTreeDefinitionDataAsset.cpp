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
#include "Data/SkillTreeDefinitionDataAsset.h"

#if WITH_EDITOR
EDataValidationResult USkillTreeNodeDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (SkillTreeNodeObjectClass.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("USkillTreeNodeDefinitionDataAsset", "", "USkillTreeNodeObject missing. No valid TSoftClassPtr specified!"));
	}

	return Result;
}
#endif

const TSoftClassPtr<USkillTreeNodeObject>& USkillTreeNodeDefinitionDataAsset::GetSkillTreeNodeObjectClass() const
{
	return SkillTreeNodeObjectClass;
}

bool USkillTreeNodeDefinitionDataAsset::CanAccessSkillTreeNodeObject(const FGameplayTagContainer& RequirementTags,
                                                                     const FGameplayTagContainer& BlockingTags) const
{
	const bool bHasRequiredTags = RequirementTags.HasAllExact(RequiredTagsForAccess);
	const bool bHasBlockingTags = BlockingTags.HasAnyExact(BlockingTagsForAccess);
	return !bHasBlockingTags && bHasRequiredTags;
}

#if WITH_EDITOR
EDataValidationResult FSkillTreeNodeObjectDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (SkillTreeObjectDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FSkillTreeNodeObjectDefinitionDataTableRow", "", "USkillTreeNodeObjectDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FSkillTreeNodeObjectDefinitionDataTableRow::GetResourcesPaths() const
{
	return {SkillTreeObjectDefinition.ToSoftObjectPath()};
}

#if WITH_EDITOR
EDataValidationResult USkillTreeDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (SkillTreeNodeIds.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("USkillTreeDefinitionDataAsset", "", "FDataRegistry Collection is Empty. No valid entries detected!"));
	}

	return Result;
}
#endif

const TArray<FDataRegistryId>& USkillTreeDefinitionDataAsset::GetSkillTreeNodeObjectIds() const
{
	return SkillTreeNodeIds;
}

#if WITH_EDITOR
EDataValidationResult FSkillTreeDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (SkillTreeDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FSkillTreeDefinitionDataTableRow", "", "USkillTreeDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FSkillTreeDefinitionDataTableRow::GetResourcesPaths() const
{
	return {SkillTreeDefinition.ToSoftObjectPath()};
}
