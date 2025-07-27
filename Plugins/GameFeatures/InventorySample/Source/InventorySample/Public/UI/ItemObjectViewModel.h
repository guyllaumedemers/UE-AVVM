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
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"

#include "ItemObjectViewModel.generated.h"

class UItemObject;
class UItemObjectViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExchangeContextChanged, const UItemObjectViewModel*, NewModifiedItem);

/**
 *	Class description:
 *
 *	UItemObjectViewModel is a view model type that provides ui information about a singular object, and it's state.
 */
UCLASS()
class INVENTORYSAMPLE_API UItemObjectViewModel : public UMVVMViewModelBase,
                                                 public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UItemObjectViewModel* Make(UItemObject* NewItem);

	virtual FName GetViewModelFName() const override { return TEXT("UItemObjectViewModel"); };

	UPROPERTY(BlueprintAssignable)
	FOnExchangeContextChanged OnExchangeContextChanged;

protected:
	void Init(UItemObject* NewItem);

	UFUNCTION()
	void OnItemRuntimeStateChanged(const FGameplayTagContainer& NewStateTags);

	UFUNCTION()
	void OnItemRuntimeCountChanged(const int32 NewCounter);

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	FGameplayTagContainer StateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	int32 Counter = 1;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UItemObject> Item = nullptr;
};
