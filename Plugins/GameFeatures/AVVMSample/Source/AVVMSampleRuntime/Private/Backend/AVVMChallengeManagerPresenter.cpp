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
#include "Backend/AVVMChallengeManagerPresenter.h"

#include "AVVM.h"
#include "AVVMOnlineInterface.h"
#include "AVVMOnlineInterfaceUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "Backend/AVVMChallengeManagerViewModel.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "GameFramework/GameMode.h"

AActor* UAVVMChallengeManagerPresenter::GetOuterKey() const
{
	return GetTypedOuter<AGameMode>();
}

void UAVVMChallengeManagerPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMChallengeManagerPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMChallengeManagerPresenter::BP_OnNotificationReceived_ForcePullChallenges(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineChallengesInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineChallengesInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMChallengeManagerPresenter::OnForcePullChallengesCompleted);

	UE_LOG(LogAVVMOnline, Log, TEXT("Force Pull Challenges Request. In-Progress..."));
	OnlineInterface->ForcePullChallenges(Callback);
}

void UAVVMChallengeManagerPresenter::BP_OnNotificationReceived_ClaimChallengeRewards(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineChallengesInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineChallengesInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMChallengeManagerPresenter::OnForcePullChallengesCompleted);

	const auto* Challenge = Payload.GetPtr<FAVVMPlayerChallenge>();
	if (Challenge != nullptr)
	{
		UE_LOG(LogAVVMOnline, Log, TEXT("Claim Challenge Rewards Request. In-Progress..."));
		OnlineInterface->ClaimChallenge(*Challenge, Callback);
	}
	else
	{
		UE_LOG(LogAVVMOnline, Log, TEXT("Claim Empty Challenge Rewards Request. In-Progress..."));
		OnlineInterface->ClaimChallenge(FAVVMPlayerChallenge{}, Callback);
	}
}

void UAVVMChallengeManagerPresenter::SetChallenges(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Updating local Challenges!"));

	auto* ChallengeManagerViewModel = Cast<UAVVMChallengeManagerViewModel>(ViewModel.Get());
	if (IsValid(ChallengeManagerViewModel))
	{
		ChallengeManagerViewModel->SetChallenges(FAVVMOnlineModule::GetJsonParser(), Payload);
	}
}

void UAVVMChallengeManagerPresenter::StartPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(this);
	if (!ensureAlwaysMsgf(IsValid(LocalPlayer),
	                      TEXT("UAVVMChallengeManagerPresenter couldn't find a valid LocalPlayer!")))
	{
		return;
	}

	FAVVMPrimaryGameLayoutContextArgs ContextArgs;
	ContextArgs.LayerTag = TargetTag;
	ContextArgs.WidgetClass = WidgetClass;
	PushContentToPrimaryGameLayout(this, LocalPlayer, ContextArgs);
}

void UAVVMChallengeManagerPresenter::StopPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(this);
	if (IsValid(LocalPlayer))
	{
		PopContentFromPrimaryGameLayout(LocalPlayer, ActivatableView.Get());
	}
}

void UAVVMChallengeManagerPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}

void UAVVMChallengeManagerPresenter::OnForcePullChallengesCompleted(const bool bWasSuccess,
                                                                    const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Force Pull Challenges Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update challenges onSuccess
		SetChallenges(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMChallengeManagerPresenter::OnClaimChallengeRewardsCompleted(const bool bWasSuccess, const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Claim Challenge Rewards Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update challenges onSuccess
		SetChallenges(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
