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
#include "Backend/AVVMOnlineInterface.h"
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

void UAVVMPartyManagerPresenter::BP_OnNotificationReceived_PullAvailableParties(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* Outer = Cast<UObject>(GetImplementingOuterObject(UAVVMOnlineInterface::StaticClass()));
	if (!ensureAlways(IsValid(Outer)))
	{
		UE_LOG(LogUI, Log, TEXT("Force Pull Parties Request. Failure! Outer doesn't Implement %s"), *UAVVMOnlineInterface::StaticClass()->GetName());
		return;
	}

	auto OnlineInterface = TScriptInterface<IAVVMOnlineInterface>(Outer);
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	IAVVMOnlineInterface::FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPartyManagerPresenter::OnForcePullPartiesCompleted);

	UE_LOG(LogUI, Log, TEXT("Force Pull Parties Request. In-Progress..."));
	OnlineInterface->ForcePullParties(Callback);
}

void UAVVMPartyManagerPresenter::SetParties(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers additional data could be defined to properly log output here!
	UE_LOG(LogUI, Log, TEXT("Updating local Parties!"));

	auto* PartyManagerViewModel = Cast<UAVVMPartyManagerViewModel>(ViewModel.Get());
	if (IsValid(PartyManagerViewModel))
	{
		PartyManagerViewModel->SetParties(Payload);
	}
}

void UAVVMPartyManagerPresenter::StartPresenting()
{
	// TODO @gdemers we do not know if the PartyManager is a View that takes in all the screen
	// or simply a portion of the user HUD, etc... TBD by design for your project needs!
	// Most Importantly, we do not know if we should push :
	//	A) On a layer Stack
	//	B) On an extension Point
}

void UAVVMPartyManagerPresenter::StopPresenting()
{
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
