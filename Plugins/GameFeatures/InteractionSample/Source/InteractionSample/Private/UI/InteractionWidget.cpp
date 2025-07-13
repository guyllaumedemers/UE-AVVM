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
#include "UI/InteractionWidget.h"

#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "UI/AVVMProgressBarWidget.h"

void UInteractionWidget::SetSlateBrush(const FSlateBrush& NewSlateBrush)
{
	if (!IsValid(InputImage))
	{
		return;
	}

	FOnLoadGuardStateChangedEvent Delegate = InputImage->OnLoadingStateChanged();
	if (!Delegate.IsBoundToObject(this))
	{
		InputImage->OnLoadingStateChanged().AddUObject(this, &UInteractionWidget::OnLoadingStateChanged);
	}

	if (!InputTexture.IsValid())
	{
		InputImage->SetBrushFromLazyDisplayAsset(NewSlateBrush.GetResourceObject());
	}

	OnLoadingStateChanged(InputImage->IsLoading());
}

void UInteractionWidget::SetProgressBarValue(const float NewValue)
{
	if (IsValid(ProgressBar))
	{
		ProgressBar->SetValue(NewValue);
	}
}

void UInteractionWidget::SetInputHoldingText(const bool bDoesRequireInputHolding)
{
	if (bDoesRequireInputHolding && IsValid(PromptText))
	{
		PromptText->SetText(InputHoldingText);
	}
}

void UInteractionWidget::SetInputMashingText(const bool bDoesRequireInputMashing)
{
	if (bDoesRequireInputMashing && IsValid(PromptText))
	{
		PromptText->SetText(InputMashingText);
	}
}

void UInteractionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(PromptText))
	{
		PromptText->SetText(InputPressText);
	}

#if WITH_EDITORONLY_DATA
	SetSlateBrush(PreviewSlateBrush);
	if (bDoesPreviewComplexInteraction)
	{
		SetInputHoldingText(bToggleTextPreview);
		SetInputMashingText(!bToggleTextPreview);
	}
#endif
}

void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(InputImage))
	{
		// @gdemers reset brush safely and load texture based on VM binding events
		InputImage->SetBrush(FSlateBrush());
		OnLoadingStateChanged(InputImage->IsLoading());
	}
}

void UInteractionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	InputTexture.Reset();
}

void UInteractionWidget::OnLoadingStateChanged(bool bIsLoading)
{
	if (bIsLoading)
	{
		return;
	}

	if (IsValid(InputImage))
	{
		InputTexture = Cast<UTexture2D>(InputImage->GetBrush().GetResourceObject());
	}
}
