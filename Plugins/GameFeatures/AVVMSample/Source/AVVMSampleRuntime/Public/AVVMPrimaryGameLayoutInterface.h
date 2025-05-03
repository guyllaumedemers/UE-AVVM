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

#include "GameplayTags.h"
#include "UObject/Interface.h"

#include "AVVMPrimaryGameLayoutInterface.generated.h"

enum class EAsyncWidgetLayerState : uint8;
class UCommonActivatableWidget;

/**
 *	Class description:
 *
 *	FAVVMPrimaryGameLayoutContextArgs encapsulate arguments to pass to the Primary
 *	Game layout.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMPrimaryGameLayoutContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag LayerTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadWrite)
	TSubclassOf<UCommonActivatableWidget> WidgetClass = nullptr;
};

/**
 *	Class description:
 *
 *	UAVVMPrimaryGameLayoutInterface define reusable api to be implemented by presenter class to handle
 *	recurrent actions like Start/Stop presenting which interface with Unreal Layering system.
*	
 *	Note : It's expected that your Presenter class only implement one of the following interface :
 *	IAVVMPrimaryGameLayoutInterface or IAVVMUIExtensionInterface.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMSAMPLERUNTIME_API UAVVMPrimaryGameLayoutInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMSAMPLERUNTIME_API IAVVMPrimaryGameLayoutInterface
{
	GENERATED_BODY()

protected:
	void PushContentToPrimaryGameLayout(UObject* Outer, const FAVVMPrimaryGameLayoutContextArgs& ContextArgs);
	void PopContentFromPrimaryGameLayout(const UObject* Outer, UCommonActivatableWidget* Target);
	void OnPushActivatableWidgetCompleted(EAsyncWidgetLayerState State, UCommonActivatableWidget* ActivatableWidget);
	virtual void BindViewModel() const PURE_VIRTUAL(BindViewModel, return;);

	TWeakObjectPtr<UCommonActivatableWidget> ActivatableView = nullptr;
};
