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

#include "CommonUserWidget.h"

#include "InteractionWidget.generated.h"

class UAVVMProgressBarWidget;
class UCommonLazyImage;
class UCommonTextBlock;
class UInputAction;

/**
 *	Class description:
 *
 *	UInteractionWidget is a widget class that provide visual feedback to available interaction during gameplay.
 */
UCLASS()
class INTERACTIONSAMPLE_API UInteractionWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetInputAction(const UInputAction* NewInputAction);

	UFUNCTION(BlueprintCallable)
	void SetProgressBarValue(const float NewValue);

	UFUNCTION(BlueprintCallable)
	void SetInputHoldingText(const bool bDoesRequireInputHolding);

	UFUNCTION(BlueprintCallable)
	void SetInputMashingText(const bool bDoesRequireInputMashing);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnLoadingStateChanged(bool bIsLoading);

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TObjectPtr<UInputAction> PreviewInputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	bool bDoesPreviewComplexInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesPreviewComplexInteraction"))
	bool bToggleTextPreview = false;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FText InputPressText = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FText InputHoldingText = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FText InputMashingText = FText();

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UAVVMProgressBarWidget> ProgressBar = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCommonLazyImage> InputImage = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> PromptText = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UTexture2D> InputTexture = nullptr;
};
