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
#include "Party/AVVMPartyManagerPresenter.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "Backend/AVVMOnlineInterface.h"
#include "Backend/AVVMOnlineInterfaceUtils.h"
#include "Backend/AVVMOnlineJsonParser.h"
#include "Party/AVVMPartyManagerViewModel.h"

AActor* UAVVMPartyManagerPresenter::GetOuterKey() const
{
	return Super::GetOuterKey();
}

void UAVVMPartyManagerPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMPartyManagerPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMPartyManagerPresenter::BP_OnNotificationReceived_ForcePullParties(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPartyManagerPresenter::OnForcePullPartiesCompleted);

	UE_LOG(LogUI, Log, TEXT("Force Pull Parties Request. In-Progress..."));
	OnlineInterface->ForcePullParties(Callback);
}

void UAVVMPartyManagerPresenter::BP_OnNotificationReceived_JoinParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPartyManagerPresenter::OnPartyJoinRequestCompleted);

	const auto* Party = Payload.GetPtr<FAVVMParty>();
	if (Party != nullptr)
	{
		UE_LOG(LogUI, Log, TEXT("Join Party Request Request. In-Progress..."));
		OnlineInterface->JoinParty(*Party, Callback);
	}
	else
	{
		UE_LOG(LogUI, Log, TEXT("Join Empty Party Request Request. In-Progress..."));
		OnlineInterface->JoinParty(FAVVMParty{}, Callback);
	}
}

void UAVVMPartyManagerPresenter::BP_OnNotificationReceived_ExitParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPartyManagerPresenter::OnPartyExitRequestCompleted);

	const auto* Party = Payload.GetPtr<FAVVMParty>();
	if (Party != nullptr)
	{
		UE_LOG(LogUI, Log, TEXT("Exit Party Request Request. In-Progress..."));
		OnlineInterface->ExitParty(*Party, Callback);
	}
	else
	{
		UE_LOG(LogUI, Log, TEXT("Exit Empty Party Request Request. In-Progress..."));
		OnlineInterface->ExitParty(FAVVMParty{}, Callback);
	}
}

void UAVVMPartyManagerPresenter::SetParties(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers additional data could be defined to properly log output here!
	UE_LOG(LogUI, Log, TEXT("Updating local Parties!"));

	auto* PartyManagerViewModel = Cast<UAVVMPartyManagerViewModel>(ViewModel.Get());
	if (IsValid(PartyManagerViewModel))
	{
		PartyManagerViewModel->SetParties(GetOuterKey(), Payload);
	}
}

void UAVVMPartyManagerPresenter::StartPresenting()
{
	FAVVMPrimaryGameLayoutContextArgs ContextArgs;
	ContextArgs.LayerTag = TargetTag;
	ContextArgs.WidgetClass = WidgetClass;
	PushContentToPrimaryGameLayout(this, ContextArgs);
}

void UAVVMPartyManagerPresenter::StopPresenting()
{
	PopContentFromPrimaryGameLayout(this, ActivatableView.Get());
}

void UAVVMPartyManagerPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}

void UAVVMPartyManagerPresenter::OnForcePullPartiesCompleted(const bool bWasSuccess,
                                                             const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Force Pull Parties Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update parties onSuccess
		SetParties(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPartyManagerPresenter::OnPartyJoinRequestCompleted(const bool bWasSuccess,
                                                             const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Join Party Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers Post-Join, we expect to broadcast to the following systems :
		//		A) HostConfigurationPresenter	- initialize our Host Configuration with payload data (i.e FAVVMParty)
		//		B) PlayerManagerPresenter		- run initial logic for creating players on HUD or other design requirements. (maybe 3d static representation)
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPartyManagerPresenter::OnPartyExitRequestCompleted(const bool bWasSuccess,
                                                             const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Exit Party Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers Post-Exit, we expect to broadcast to the following systems :
		//		A) HostConfigurationPresenter	- reset our Host Configuration with payload data (i.e FAVVMParty)(now expected to be default/empty)
		//		B) PlayerManagerPresenter		- run initial logic for destroying players on HUD or other design requirements. (maybe 3d static representation)
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
