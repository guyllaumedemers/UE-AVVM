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

#include "AVVMMultiContextWindowWidget.h"
#include "Engine/StreamableManager.h"
#include "Templates/SubclassOf.h"

#include "AVVMFloatingMultiContextWindowWidget.generated.h"

class UAVVMWidgetPickerDataAsset;
class UCanvasPanel;

/**
 *	Class description:
 *
 *	UAVVMFloatingWindowBehaviourImpl is an abstract around the implementation details of a Widget behavior. Derived type
 *	for supporting behavior such as Drag/Drop, Docking and Minimized are expected!
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMFloatingWindowBehaviourImpl : public UObject
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportTick() const PURE_VIRTUAL(DoesSupportTick, return false;)
};

/**
 *	Class description:
 *
 *	UAVVMFloatingMultiContextWindowWidget is the Floating version of the MultiContextWindowWidget that define
 *	a free form layout with multiple context. Context Window can be opened, closed and moved around.
 *
 *	Example : Diablo Inventory system.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMFloatingMultiContextWindowWidget : public UAVVMMultiContextWindowWidget
{
	GENERATED_BODY()

protected:
	virtual void SetupWindows_Internal(TArray<UObject*> NewViewModels) override;
	virtual void AddWindow_Internal(UObject* NewViewModel) override;
	virtual void RemoveWindow_Internal(UObject* NewViewModel) override;
	
	bool HasWidgetClass();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UAVVMFloatingWindowBehaviourImpl>> FloatingBehaviourClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMWidgetPickerDataAsset> WidgetPickerDataAsset = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> FloatingWindowRoot = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UAVVMFloatingWindowBehaviourImpl>> FloatingBehaviours;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};
