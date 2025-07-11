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

#include "AVVMStaticMultiContextWindowWidget.generated.h"

class UAVVMDynamicEntryBoxExtendedWidget;

/**
 *	Class description:
 *
 *	UAVVMStaticMultiContextWindowWidget is the Static version of the MultiContextWindowWidget that define
 *	a fix layout with multiple context. Context Window can still be opened and closed but are anchored to defined anchor.
 *
 *	Example : Crafting system where user has to drag from items from left to right and stack them to output a new Item.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMStaticMultiContextWindowWidget : public UAVVMMultiContextWindowWidget
{
	GENERATED_BODY()

protected:
	virtual void SetupWindows_Internal(TArray<UObject*> NewViewModels) override;
	virtual void AddWindow_Internal(UObject* NewViewModel) override;
	virtual void RemoveWindow_Internal(UObject* NewViewModel) override;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UAVVMDynamicEntryBoxExtendedWidget> StaticWindowRoot = nullptr;
};
