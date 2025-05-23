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
#include "InteractionAbilityData.h"

#if WITH_EDITOR
EDataValidationResult UInteractionAbilityDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (!IsValid(ActivationInfo.PreviewInteractionWidgetClass) || !IsValid(TransitionInfo.WidgetClass) || TransitionInfo.CameraTransitionVfx.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UInteractionAbilityDataAsset", "", "Missing TSubClassOf<T>, or TSoftObjectPtr<T>!"));
	}

	return Result;
}
#endif

const FInteractionAbilityActivationInfo& UInteractionAbilityDataAsset::GetActivationInfo() const
{
	return ActivationInfo;
}

const FInteractionAbilityTransitionInfo& UInteractionAbilityDataAsset::GetTransitionInfo() const
{
	return TransitionInfo;
}
