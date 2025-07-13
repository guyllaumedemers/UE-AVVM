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

#include "AVVMFrameWidget.h"

#include "AVVMFloatingFrameWidget.generated.h"

class UCanvasPanel;

/**
 *	Class description:
 *
 *	UAVVMFloatingFrameWidget is a widget class that can holds many UAVVMStaticFrameWidget. It references
 *	the anchor to with elements can be child.
 *
 *	How to use ?
 *
 *		* There should always be one Outer most UAVVMFloatingFrameWidget that exists on the CommonActivatable Widget
 *		that's active.
 *
 *		* Based on the View Model received (likely provided from user opening UI system context), we create a UAVVMStaticFrameWidget
 *		that we anchor to our Outer most.
 *
 *		* Our child UAVVMStaticFrameWidget may be a complex element, built with many subdivisions which could support being detached. If so,
 *		detaching a subdivision should be handled and anchored to our Outer most UAVVMFloatingFrameWidget.
 *
 *	Example : Guild Wars (1 - not 2, the second is garbage. For Ascalon!) UI system allowed opening many contexts windows on screen
 *	such as minimap, inventory, achievements, etc... The inventory bags provided a unique context that could be detached.
 */
UCLASS(Blueprintable)
class AVVMTOOLKIT_API UAVVMFloatingFrameWidget : public UAVVMFrameWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void SetupWindows_Internal(TArray<UObject*> NewViewModels) override;
	virtual void AddWindow_Internal(UObject* NewViewModel) override;
	virtual void RemoveWindow_Internal(UObject* NewViewModel) override;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCanvasPanel> Root = nullptr;
};
