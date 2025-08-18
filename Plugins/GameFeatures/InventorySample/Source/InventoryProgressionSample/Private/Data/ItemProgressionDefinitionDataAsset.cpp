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
#include "Data/ItemProgressionDefinitionDataAsset.h"

#if WITH_EDITOR
EDataValidationResult UItemProgressionStageDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ProgressionGameplayEffects.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UItemProgressionStageDefinitionDataAsset", "", "Progression Effects missing. No valid GameplayEffect specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> UItemProgressionStageDefinitionDataAsset::GetProgressionStackingEffectSoftObjectPaths() const
{
	TArray<FSoftObjectPath> OutResults;
	for (const TSoftClassPtr<UGameplayEffect>& GameplayEffectClass : ProgressionGameplayEffects)
	{
		OutResults.Add(GameplayEffectClass.ToSoftObjectPath());
	}

	return OutResults;
}

#if WITH_EDITOR
EDataValidationResult UItemProgressionDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (bDoesApplyProgressionEffects && ItemProgressionStageDataAssets.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UItemProgressionDefinitionDataAsset", "", "Progression Stages missing. No valid entries specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> UItemProgressionDefinitionDataAsset::GetProgressionStages(const int32 ProgressionStageIndex) const
{
	if (ProgressionStageIndex == INDEX_NONE)
	{
		return TArray<FSoftObjectPath>{};
	}

	TArray<FSoftObjectPath> OutResults;
	for (const auto& [StageLevel, ProgressionSoftObjectPtr] : ItemProgressionStageDataAssets)
	{
		if (StageLevel <= ProgressionStageIndex)
		{
			OutResults.Add(ProgressionSoftObjectPtr.ToSoftObjectPath());
		}
	}

	return bDoesApplyProgressionEffects ? OutResults : TArray<FSoftObjectPath>{};
}

#if WITH_EDITOR
EDataValidationResult FItemProgressionDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ItemProgressionDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMItemProgressionDefinitionDataTableRow", "", "UItemProgressionDefinitionDataAsset missing. No valid TSoftObjectPtr specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FItemProgressionDefinitionDataTableRow::GetResourcesPaths() const
{
	return {ItemProgressionDefinition.ToSoftObjectPath()};
}
