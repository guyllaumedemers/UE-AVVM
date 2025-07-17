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
#include "Data/AVVMHandshakePayload.h"

#include "MultiContextInventoryViewModel.generated.h"

class UInventoryContextViewModel;
class UItemObject;

/**
 *	Class description:
 *
 *	FExchangeContext is a struct that cache information about the inventory system we are acting on. This
 *	can be a single instance or multiple. (example : during a trade between players)
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FExchangeContext
{
	GENERATED_BODY()

	FExchangeContext() = default;
	FExchangeContext(const FAVVMHandshakePayload* NewPayload);
	bool operator==(const FExchangeContext& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	TObjectPtr<const UInventoryContextViewModel> Src = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	TObjectPtr<const UInventoryContextViewModel> Dest = nullptr;
};

/**
 *	TBD
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryContextViewModel : public UMVVMViewModelBase,
                                                       public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UInventoryContextViewModel* Make(const TArray<UItemObject*>& NewItems);

	virtual FName GetViewModelFName() const override { return TEXT("UInventoryContextViewModel"); };

	void Init(const TArray<UItemObject*>& NewItems);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TArray<UItemObject*> Items;
};

/**
 *	Class description:
 *
 *	UMultiContextInventoryViewModel is a view model type that provide ui information about the holder inventory
 *	and optionally, the end point user interacting with it.
 */
UCLASS()
class INVENTORYSAMPLE_API UMultiContextInventoryViewModel : public UMVVMViewModelBase,
                                                            public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	virtual FName GetViewModelFName() const override { return TEXT("UMultiContextInventoryViewModel"); };
	virtual void SetPayload(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	FExchangeContext ExchangeContext = FExchangeContext();
};
