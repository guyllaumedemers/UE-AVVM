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

#include "ExchangeContextViewModel.generated.h"

class UItemObject;
class UItemObjectViewModel;

/**
 *	Class description:
 *
 *	FExchangeContextState is a POD class that define the ui state of the running exchange. This simply tracks information
 *	and updates ui elements.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FExchangeContextState
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	UExchangeContextViewModel is an Abstract view model type that provides ui information about an action between two end users or
 *	between a player and itself.
 *
 *	Example : During a trade, player A selects the items to be put under the "trade" window (shared by both players) and awaits confirmation
 *	or acceptance from player B.
 *
 *	Other cases could involve a single player equipping gear from their inventory onto the character available preview slots.
 *
 *	Derived view model type should be added for specific actions.
 *
 *	Example : UTradeExchangeContextViewModel, UConsumptionExchangeViewModel, etc... The creation of one of these view model contexts
 *	would most likely involve pushing a prompt on screen to confirm any pending action.
 */
UCLASS()
class INVENTORYSAMPLE_API UExchangeContextViewModel : public UMVVMViewModelBase,
                                                      public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UExchangeContextViewModel* Make(const TArray<UItemObject*>& NewItems);

	virtual FName GetViewModelFName() const override { return TEXT("UExchangeContextViewModel"); }
	virtual void Process(const UItemObjectViewModel* NewModifiedItem, const bool bIsOwnedBySrc = true) PURE_VIRTUAL(Process, return;);

protected:
	void Init(const TArray<UItemObject*>& NewItems);

	UFUNCTION()
	void OnItemChanged(const UItemObjectViewModel* NewModifiedItem);

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TArray<TObjectPtr<const UItemObjectViewModel>> ItemViewModels;

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TArray<TObjectPtr<const UItemObjectViewModel>> PendingItemViewModels;

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	FExchangeContextState ExchangeContextState = FExchangeContextState();

	// TODO @gdemers complex cases where singular context representing a player could drag/drop around items between sections
	// of a more complex ui. example : from backpack to equip slot armor. a prompt could display the items context for swaping.
	// To be further defined!
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TObjectPtr<UExchangeContextViewModel> NestedContext = nullptr;
};
