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
#include "Ability/AVVMAbilityData.h"

#if WITH_EDITOR
EDataValidationResult UAVVMAbilityDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	const bool bIsInputActionValid = !bIsPassiveAbility ? !AbilityInputAction.IsNull() : true;
	if (!bIsInputActionValid || GameplayAbility.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMAbilityDataAsset", "", "No valid TSoftClassPtr<T> specified!"));
	}

	return Result;
}
#endif

bool UAVVMAbilityDataAsset::CanGrantAbility(const FGameplayTagContainer& ContextualTags) const
{
	return true;
}

const TSoftClassPtr<UGameplayAbility>& UAVVMAbilityDataAsset::GetGameplayAbilityClass() const
{
	return GameplayAbility;
}

const TSoftObjectPtr<UInputAction>& UAVVMAbilityDataAsset::GetInputAction() const
{
	return AbilityInputAction;
}

#if WITH_EDITOR
EDataValidationResult FAVVMAbilityDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (AbilityDataAsset.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMAbilityDataTableRow", "", "No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FAVVMAbilityDataTableRow::GetResourcesPaths() const
{
	return {AbilityDataAsset.ToSoftObjectPath()};
}

const TArray<FDataRegistryId>& UAVVMAbilityGroupDataAsset::GetAbilities() const
{
	return Abilities;
}

TArray<FSoftObjectPath> FAVVMAbilityGroupDataTableRow::GetResourcesPaths() const
{
	TArray<FSoftObjectPath> OutPaths;
	for (const TSoftObjectPtr<UAVVMAbilityGroupDataAsset>& AbilityGroup : AbilityGroupDataAssets)
	{
		OutPaths.AddUnique(AbilityGroup.ToSoftObjectPath());
	}

	return OutPaths;
}
