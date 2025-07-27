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

#include "AVVM.h"
#include "MVVMViewModelBase.h"

#include "SingleContextInventoryViewModel.generated.h"

class UItemObject;
class UItemObjectViewModel;

/**
 *	Class description:
 *
 *	USingleContextInventoryViewModel is a view model type that store data about the owner items.
 */
UCLASS()
class INVENTORYSAMPLE_API USingleContextInventoryViewModel : public UMVVMViewModelBase,
                                                             public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static USingleContextInventoryViewModel* Make(const TArray<UItemObject*>& NewItems,
	                                              ULocalPlayer* NewLocalPlayer);

	virtual FName GetViewModelFName() const override { return TEXT("USingleContextInventoryViewModel"); }
	void Process(const UItemObjectViewModel* NewModifiedItem);

protected:
	void Init(const TArray<UItemObject*>& NewItems);

	UFUNCTION()
	void OnItemChanged(const UItemObjectViewModel* NewModifiedItem);

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TArray<TObjectPtr<UItemObjectViewModel>> ItemViewModels;
};
