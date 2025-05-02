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
#include "AVVMSampleRuntime/Public/Backend/AVVMAccountLoginPresenter.h"

#include "AVVM.h"
#include "AVVMGameMode.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "MVVMViewModelBase.h"
#include "PrimaryGameLayout.h"

AActor* UAVVMAccountLoginPresenter::GetOuterKey() const
{
	return GetTypedOuter<AAVVMGameMode>();
}

void UAVVMAccountLoginPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMAccountLoginPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMAccountLoginPresenter::BP_OnNotificationReceived_TryLogin(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* Outer = Cast<UObject>(GetImplementingOuterObject(UAVVMOnlineInterface::StaticClass()));
	if (!ensureAlways(IsValid(Outer)))
	{
		UE_LOG(LogUI, Log, TEXT("Login Request. Failure! Outer doesn't Implement %s"), *UAVVMOnlineInterface::StaticClass()->GetName());
		return;
	}

	// @gdemers BP implemented interface cannot be resolved when Cast<> or TScriptInterface::Ctor are called and will return null. Interface support
	// is required natively!
	auto OnlineInterface = TScriptInterface<IAVVMOnlineInterface>(Outer);
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	IAVVMOnlineInterface::FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMAccountLoginPresenter::OnLoginRequestCompleted);

	const auto* LoginContextData = Payload.GetPtr<FAVVMLoginContextData>();
	if (LoginContextData != nullptr)
	{
		UE_LOG(LogUI, Log, TEXT("Sending Login Request. In-Progress..."));
		OnlineInterface->RequestLogin(*LoginContextData, Callback);
	}
	else
	{
		UE_LOG(LogUI, Log, TEXT("Sending Empty Login Request. In-Progress..."));
		OnlineInterface->RequestLogin(FAVVMLoginContextData{}, Callback);
	}
}

void UAVVMAccountLoginPresenter::StartPresenting()
{
	const auto Callback = [Caller = TWeakObjectPtr(this)](EAsyncWidgetLayerState State,
	                                                      UCommonActivatableWidget* ActivatableWidget)
	{
		if (Caller.IsValid())
		{
			Caller->OnPresenterStartCompleted(State, ActivatableWidget);
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

	UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
	if (ensure(IsValid(GameLayout)))
	{
		GameLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(TargetTag,
		                                                                  true,
		                                                                  WidgetClass.Get(),
		                                                                  Callback);
	}
}

void UAVVMAccountLoginPresenter::StopPresenting()
{
	UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
	if (ensure(IsValid(GameLayout)))
	{
		GameLayout->FindAndRemoveWidgetFromLayer(PresentingWidget.Get());
	}
}

void UAVVMAccountLoginPresenter::OnPresenterStartCompleted(EAsyncWidgetLayerState State,
                                                           UCommonActivatableWidget* ActivatableWidget)
{
	if (!IsValid(ActivatableWidget) || State != EAsyncWidgetLayerState::AfterPush)
	{
		return;
	}

	UE_LOG(LogUI, Log, TEXT("Presenting %s"), *ActivatableWidget->GetName());
	PresentingWidget = ActivatableWidget;

	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableWidget);
}

void UAVVMAccountLoginPresenter::OnLoginRequestCompleted(const bool bWasSuccess,
                                                         const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Login Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers our received payload should aggregate our player representation
		// and allow notification to the PlayerProfilePresenter, initialize and present however design expect.
		BP_OnLoginRequestSuccess(Payload);
	}
	else
	{
		BP_OnLoginRequestFailure(Payload);
	}
}
