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
#include "Data/ItemDefinitionDataAsset.h"

#if WITH_EDITOR
EDataValidationResult UItemDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ItemObjectClass.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UItemDefinitionDataAsset", "", "UItemObject missing. No valid TSoftClassPtr specified!"));
	}

	return Result;
}
#endif

const TSoftClassPtr<UItemObject>& UItemDefinitionDataAsset::GetItemObjectClass() const
{
	return ItemObjectClass;
}

bool UItemDefinitionDataAsset::CanAccessItem(const FGameplayTagContainer& RequirementTags,
                                             const FGameplayTagContainer& BlockingTags) const
{
	const bool bMeetRequirements = RequiredTagsForItemAccess.IsEmpty() || RequirementTags.HasAllExact(RequiredTagsForItemAccess);
	const bool bIsntBlocked = BlockingTagsForItemAccess.IsEmpty() || !BlockingTags.HasAnyExact(BlockingTagsForItemAccess);
	return bIsntBlocked && bMeetRequirements;
}

#if WITH_EDITOR
EDataValidationResult FItemDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ItemDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMItemDefinitionDataTableRow", "", "UItemDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FItemDefinitionDataTableRow::GetResourcesPaths() const
{
	return {ItemDefinition.ToSoftObjectPath()};
}

#if WITH_EDITOR
EDataValidationResult UItemGroupDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ItemIds.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UItemCollectionDefinitionDataAsset", "", "FDataRegistry Collection is Empty. No valid entries detected!"));
	}

	return Result;
}

const TArray<FDataRegistryId>& UItemGroupDefinitionDataAsset::GetItemIds() const
{
	return ItemIds;
}

EDataValidationResult FItemGroupDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ItemGroupDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMItemCollectionDefinitionDataTableRow", "", "UItemCollectionDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FItemGroupDefinitionDataTableRow::GetResourcesPaths() const
{
	return {ItemGroupDefinition.ToSoftObjectPath()};
}
