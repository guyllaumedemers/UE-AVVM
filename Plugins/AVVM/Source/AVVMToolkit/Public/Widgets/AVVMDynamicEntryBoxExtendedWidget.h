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
#include "Engine/StreamableManager.h"
#include "Templates/SubclassOf.h"

#include "AVVMDynamicEntryBoxExtendedWidget.generated.h"

class UAVVMWidgetPickerDataAsset;
class UDynamicEntryBox;

/**
 *	Class description:
 *
 *	UAVVMDynamicEntryBoxExtendedWidget is a Widget class that define an api covering basic use cases when interfacing with DynamicEntryBox. It extend the basic api
 *	and make use of the MVVM plugin to provide data to dynamic entries.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMDynamicEntryBoxExtendedWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(TArray<UObject*> NewViewModels);

	UFUNCTION(BlueprintCallable)
	void Add(const TSubclassOf<UCommonUserWidget>& NewWidgetClass,
	         UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void RemoveEntry(UCommonUserWidget* NewWidget);

	bool HasWidgetClass() const;

protected:
	virtual void NativeDestruct() override;
	void GetDeferredWidgetClass(const FSoftObjectPath& WidgetClassPicker,
	                            const TArray<UObject*>& NewViewModels);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMWidgetPickerDataAsset> WidgetPickerDataAsset = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UDynamicEntryBox> DynamicEntryBox = nullptr;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};
