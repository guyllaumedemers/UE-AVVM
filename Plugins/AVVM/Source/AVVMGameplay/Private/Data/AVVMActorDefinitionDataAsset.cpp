﻿//Copyright(c) 2025 gdemers
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
#include "Data/AVVMActorDefinitionDataAsset.h"

#if WITH_EDITOR
EDataValidationResult UAVVMActorDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (bDoesSupportPassiveAbilities && !PassiveAbilityGroupId.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMActorDefinitionDataAsset", "", "PassiveAbilityGroupId invalid!"));
	}

	if (bDoesSupportActiveAbilities && !ActiveAbilityGroupId.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMActorDefinitionDataAsset", "", "ActiveAbilityGroupId invalid!"));
	}

	if (bDoesSupportCosmetic && !CosmeticDefinitionId.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMActorDefinitionDataAsset", "", "CosmeticDefinitionId invalid!"));
	}

	if (bDoesSupportEquipment && !EquipmentDefinitionId.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMActorDefinitionDataAsset", "", "EquipmentDefinitionId invalid!"));
	}

	return Result;
}

EDataValidationResult FAVVMActorDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ActorDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMActorDefinitionDataTableRow", "", "UAVVMActorDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FAVVMActorDefinitionDataTableRow::GetResourcesPaths() const
{
	return {ActorDefinition.ToSoftObjectPath()};
}
