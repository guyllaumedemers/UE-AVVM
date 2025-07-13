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
 *	UAVVMStaticFrameWidget is a widget class that can holds many UAVVMWindowWidget. It references
 *	a grid-like container type to with elements can be child.
 *
*	How to use ?
 *
*		* Based on the View Model received (likely provided from the Parent View Model sub-ViewModels), we create a set of UAVVMWindowWidget
 *		that child to our container.
 *
 *		* Our child UAVVMWindowWidget may require specific anchoring. Being able to retrieve it's expected position in regards to existing neighbors is of utmost importance.
 *		Floating elements may be targeted as anchor.
 *
 *	Notes : Being able to anchor from a position outside our control should be taken into consideration.
 */
UCLASS(Blueprintable)
class AVVMTOOLKIT_API UAVVMStaticFrameWidget : public UAVVMFrameWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void SetupWindows_Internal(TArray<UObject*> NewViewModels) override;
	virtual void AddWindow_Internal(UObject* NewViewModel) override;
	virtual void RemoveWindow_Internal(UObject* NewViewModel) override;

	virtual void RegisterChild_Internal(const UObject* NewViewModel, const FFrameZOrder& NewZOrder) const override;
	virtual void UnRegisterChild_Internal(const UObject* NewViewModel) const override;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UDynamicEntryBox> Root = nullptr;
};
