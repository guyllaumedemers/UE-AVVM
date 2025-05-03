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
#include "AVVMPrimaryGameLayoutInterface.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "PrimaryGameLayout.h"

void IAVVMPrimaryGameLayoutInterface::PushContentToPrimaryGameLayout(UObject* Outer, const FAVVMPrimaryGameLayoutContextArgs& ContextArgs)
{
	const auto Callback = [Caller = TScriptInterface<IAVVMPrimaryGameLayoutInterface>(Outer)](EAsyncWidgetLayerState State,
	                                                                                          UCommonActivatableWidget* ActivatableWidget)
	{
		const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(Caller);
		if (bIsValid)
		{
			Caller->OnPushWidgetCompleted(State, ActivatableWidget);
		}
		else
		{
			UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(ActivatableWidget);
			if (ensure(IsValid(GameLayout)))
			{
				GameLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
			}
		}
	};

	UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(Outer);
	if (ensure(IsValid(GameLayout)))
	{
		GameLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(ContextArgs.LayerTag,
		                                                                  true,
		                                                                  ContextArgs.WidgetClass.Get(),
		                                                                  Callback);
	}
}

void IAVVMPrimaryGameLayoutInterface::PopContentToPrimaryGameLayout(UObject* Outer,
                                                                    UCommonActivatableWidget* Target)
{
	UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(Outer);
	if (ensure(IsValid(GameLayout)))
	{
		GameLayout->FindAndRemoveWidgetFromLayer(Target);
	}
}

void IAVVMPrimaryGameLayoutInterface::OnPushWidgetCompleted(EAsyncWidgetLayerState State,
                                                            UCommonActivatableWidget* ActivatableWidget)
{
	if (!IsValid(ActivatableWidget) || State != EAsyncWidgetLayerState::AfterPush)
	{
		return;
	}

	UE_LOG(LogUI, Log, TEXT("Presenting %s"), *ActivatableWidget->GetName());
	ActivatableView = ActivatableWidget;
	BindViewModel();
}
