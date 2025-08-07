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
#include "Data/AttachmentDefinitionDataAsset.h"

#if WITH_EDITOR
EDataValidationResult UAttachmentModifierDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (bDoesSupportModifiers && ModifierEffectClasses.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAttachmentModifierDefinitionDataAsset", "", "ModifierEffectClasses Empty!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> UAttachmentModifierDefinitionDataAsset::GetModifiersSoftObjectPaths() const
{
	TArray<FSoftObjectPath> OutResults;
	for (const auto& ModifierEffectClass : ModifierEffectClasses)
	{
		OutResults.Add(ModifierEffectClass.ToSoftObjectPath());
	}

	return OutResults;
}

#if WITH_EDITOR
EDataValidationResult FAttachmentModifierDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (AttachmentModifierDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAttachmentModifierDefinitionDataTableRow", "", "UAttachmentModifierDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FAttachmentModifierDefinitionDataTableRow::GetResourcesPaths() const
{
	return {AttachmentModifierDefinition.ToSoftObjectPath()};
}

#if WITH_EDITOR
EDataValidationResult UAttachmentDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (TriggeringAttachmentClass.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAttachmentDefinitionDataAsset", "", "AttachmentActorClasses Empty!"));
	}

	return Result;
}
#endif

const TSoftClassPtr<ATriggeringAttachmentActor>& UAttachmentDefinitionDataAsset::GetTriggeringAttachmentClass() const
{
	return TriggeringAttachmentClass;
}

bool UAttachmentDefinitionDataAsset::CanAccessItem(const FGameplayTagContainer& RequirementTags,
                                                   const FGameplayTagContainer& BlockingTags) const
{
	const bool bHasRequiredTags = RequirementTags.HasAllExact(RequiredTagsForItemAccess);
	const bool bHasBlockingTags = BlockingTags.HasAnyExact(BlockingTagsForItemAccess);
	return !bHasBlockingTags && bHasRequiredTags;
}

#if WITH_EDITOR
EDataValidationResult FAttachmentDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (AttachmentDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAttachmentDefinitionDataTableRow", "", "UAttachmentDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FAttachmentDefinitionDataTableRow::GetResourcesPaths() const
{
	return {AttachmentDefinition.ToSoftObjectPath()};
}
