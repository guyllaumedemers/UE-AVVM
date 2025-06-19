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
#include "Ability/AVVMAbilityDefinitionDataAsset.h"

#if WITH_EDITOR
EDataValidationResult UAVVMAbilityDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (GameplayAbilityClass.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMAbilityDataAsset", "", "GameplayAbility missing. No valid TSoftClassPtr<T> specified!"));
	}

	return Result;
}
#endif

bool UAVVMAbilityDefinitionDataAsset::CanGrantAbility(const FGameplayTagContainer& ContextualTags) const
{
	return true;
}

const TSoftClassPtr<UGameplayAbility>& UAVVMAbilityDefinitionDataAsset::GetGameplayAbilityClass() const
{
	return GameplayAbilityClass;
}

#if WITH_EDITOR
EDataValidationResult FAVVMAbilityDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (AbilityDefinitionDataAsset.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMAbilityDataTableRow", "", "UAVVMAbilityDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FAVVMAbilityDefinitionDataTableRow::GetResourcesPaths() const
{
	return {AbilityDefinitionDataAsset.ToSoftObjectPath()};
}

const TArray<FDataRegistryId>& UAVVMAbilityGroupDefinitionDataAsset::GetAbilityIds() const
{
	return AbilityIds;
}

TArray<FSoftObjectPath> FAVVMAbilityGroupDefinitionDataTableRow::GetResourcesPaths() const
{
	return {AbilityGroupDefinitionDataAsset.ToSoftObjectPath()};
}
