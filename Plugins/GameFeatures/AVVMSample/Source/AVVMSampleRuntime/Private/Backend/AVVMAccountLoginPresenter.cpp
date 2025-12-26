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
#include "Backend/AVVMAccountLoginPresenter.h"

#include "AVVM.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMOnlineInterface.h"
#include "AVVMOnlineUtils.h"
#include "AVVMUtils.h"
#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "GameFramework/GameMode.h"

AActor* UAVVMAccountLoginPresenter::GetOuterKey() const
{
	return GetTypedOuter<AGameMode>();
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
	TScriptInterface<IAVVMOnlineIdentityInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineUtils::GetOuterOnlineIdentityInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMAccountLoginPresenter::OnLoginRequestCompleted);

	const auto* LoginContext = Payload.GetPtr<FAVVMPlayerLoginContext>();
	if (LoginContext != nullptr)
	{
		UE_LOG(LogAVVMOnline, Log, TEXT("Sending Login Request. In-Progress..."));
		OnlineInterface->RequestLogin(*LoginContext, Callback);
	}
	else
	{
		UE_LOG(LogAVVMOnline, Log, TEXT("Sending Empty Login Request. In-Progress..."));
		OnlineInterface->RequestLogin(FAVVMPlayerLoginContext{}, Callback);
	}
}

void UAVVMAccountLoginPresenter::StartPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtils::GetFirstOrTargetLocalPlayer(this);
	if (!ensureAlwaysMsgf(IsValid(LocalPlayer),
	                      TEXT("UAVVMAccountLoginPresenter couldn't find a valid LocalPlayer!")))
	{
		return;
	}

	FAVVMPrimaryGameLayoutContextArgs ContextArgs;
	ContextArgs.LayerTag = TargetTag;
	ContextArgs.WidgetClass = WidgetClass;
	PushContentToPrimaryGameLayout(this, LocalPlayer, ContextArgs);
}

void UAVVMAccountLoginPresenter::StopPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtils::GetFirstOrTargetLocalPlayer(this);
	if (IsValid(LocalPlayer))
	{
		PopContentFromPrimaryGameLayout(LocalPlayer, ActivatableView.Get());
	}
}

void UAVVMAccountLoginPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtils::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}

void UAVVMAccountLoginPresenter::OnLoginRequestCompleted(const bool bWasSuccess,
                                                         const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Login Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers Post-login, we expect to broadcast to the following systems :
		//		A) PlayerProfilePresenter		- initializing our profile with payload data from login callback. i.e this call!
		//		B) PartyManagerPresenter		- run an initial request to join a new party
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
