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

#include "AVVMStaticFrameWidget.generated.h"

class UDynamicEntryBox;

/**
 *	Class description:
 *
 *	UAVVMStaticFrameWidget is a widget class that can holds many UAVVMWindowWidget (Frame encapsulating your feature). It can be parented to a Floating Frame
 *	or nested with a Static Frame. There's two case scenarios in which you want to use this Widget type.
 *
 *		A) For dynamic usage where child content needs to be dynamically created and fill a grid-like container without offering
 *		scrolling capabilities.
 *
 *		B) For static usage where your feature has explicit property child to be bound in the View Model Property binding editor. In this case,
 *		calling SetupFrames or AddFrame is irrelevant. The View Model bound should have explicit View Model properties to be directly reference
 *		through the property editor.
 */
UCLASS(Blueprintable)
class AVVMTOOLKIT_API UAVVMStaticFrameWidget : public UAVVMFrameWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void SetupFrames_Internal(TArray<UObject*> NewViewModels) override;
	virtual void AddFrame_Internal(UObject* NewViewModel) override;
	virtual void RemoveFrame_Internal(UObject* NewViewModel) override;

	virtual void RegisterChild_Internal(UObject* NewViewModel, const FFrameZOrder& NewZOrder) override;
	virtual void UnRegisterChild_Internal(UObject* NewViewModel) override;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UDynamicEntryBox> Root = nullptr;
};
