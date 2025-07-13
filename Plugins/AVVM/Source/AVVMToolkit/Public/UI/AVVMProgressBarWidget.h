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

#include "AVVMProgressBarWidget.generated.h"

class UCommonLazyImage;
class UMaterialInstance;

/**
 *	Class description:
 *
 *	UAVVMProgressBarWidget is a utility widget class that expose material properties to an image
 *	and simulate a progress bar.
 *
 *	Why do this ? UProgressBar doesn't support circular progression motion. Using materials, we can provide
 *	various styles with progression value that can be lerp through the exposed material parameters.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMProgressBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetValue(const float NewValue);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SynchronizeProperties() override;

	UFUNCTION()
	void OnLoadingStateChanged(bool bIsLoading);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayOnConstruct() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UMaterialInstance> MaterialInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FName MaterialParameterName = NAME_None;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(UIMin=0.f, UIMax=1.f))
	float PreviewValue = 0.f;
#endif

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCommonLazyImage> MaterialImage = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial = nullptr;
};
