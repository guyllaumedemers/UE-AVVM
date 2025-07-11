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

#include "AVVMMultiContextWindowWidget.generated.h"

class UCommonUserWidget;

/**
 *	Class description:
 *
 *	FWindowZOrder is a POD that cache data about the window context and the layer on which it lives.
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FWindowZOrder
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UCommonUserWidget> Window;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ZOrder = INDEX_NONE;
};

/**
 *	Class description:
 *
 *	UAVVMWindowDecorator is an abstract around the implementation details of a Widget behavior.
 *	Derived types such as Drag/Drop, Docking and Minimized are expected!
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMWindowDecorator : public UObject
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportTick() const PURE_VIRTUAL(DoesSupportTick, return false;)
	virtual void Tick(const float NewDeltaTime) PURE_VIRTUAL(Tick, return;);
};

/**
 *	Class description:
 *
 *	UAVVMMultiContextWindowWidget is an Abstract class from which we can instance and destroy context window. This widget should be referenced
 *	on Views where multiple segments should be display.
 *
 *	Example : Diablo 1 Inventory system with side by side shop. (Derived class are available for Static and Floating Context)
 */
UCLASS(Abstract, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMMultiContextWindowWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupWindows(const TArray<UObject*>& NewViewModels);

	UFUNCTION(BlueprintCallable)
	void AddWindow(UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void RemoveWindow(UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void CloseAllWindows();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SetupWindows_Internal(TArray<UObject*> NewViewModels) PURE_VIRTUAL(SetupWindows_Internal, return;);
	virtual void AddWindow_Internal(UObject* NewViewModel) PURE_VIRTUAL(AddWindow_Internal, return;);
	virtual void RemoveWindow_Internal(UObject* NewViewModel) PURE_VIRTUAL(RemoveWindow_Internal, return;);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UAVVMWindowDecorator>> WindowDecoratorClasses;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UAVVMWindowDecorator>> WindowDecorators;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<UObject>, FWindowZOrder> WindowContexts;
};
