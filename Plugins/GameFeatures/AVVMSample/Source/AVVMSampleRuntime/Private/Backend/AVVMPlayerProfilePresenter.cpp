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
#include "Backend/AVVMPlayerProfilePresenter.h"

#include "AVVM.h"
#include "AVVMGameMode.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "Backend/AVVMPlayerProfileViewModel.h"

AActor* UAVVMPlayerProfilePresenter::GetOuterKey() const
{
	return GetTypedOuter<AAVVMGameMode>();
}

void UAVVMPlayerProfilePresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();

	// @gdemers we are now logged in and have received notification from the UAccountLoginPresenter that the local Player
	// has successfully connected to their account!
	SetPlayerProfile(Payload);
}

void UAVVMPlayerProfilePresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMPlayerProfilePresenter::BP_OnNotificationReceived_CommitModifiedPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* Outer = Cast<UObject>(GetImplementingOuterObject(UAVVMOnlineInterface::StaticClass()));
	if (!ensureAlways(IsValid(Outer)))
	{
		UE_LOG(LogUI, Log, TEXT("Committing Player Profile Request. Failure! Outer doesn't Implement %s"), *UAVVMOnlineInterface::StaticClass()->GetName());
		return;
	}

	auto OnlineInterface = TScriptInterface<IAVVMOnlineInterface>(Outer);
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	IAVVMOnlineInterface::FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnCommitPlayerProfileCompleted);

	// @gdemers we obviously wouldnt commit an empty player account. we expect the online subsystem to run validation on the account
	// before executing any request to overwrite the backend data.
	const auto* PlayerProfile = Payload.GetPtr<FAVVMPlayerProfile>();
	if (PlayerProfile != nullptr)
	{
		UE_LOG(LogUI, Log, TEXT("Committing Player Profile Request. In-Progress..."));
		OnlineInterface->CommitModifiedPlayerProfile(*PlayerProfile, Callback);
	}
	else
	{
		UE_LOG(LogUI, Log, TEXT("Committing Empty Player Profile Request. In-Progress..."));
		OnlineInterface->CommitModifiedPlayerProfile(FAVVMPlayerProfile{}, Callback);
	}
}

void UAVVMPlayerProfilePresenter::BP_OnNotificationReceived_ForcePullPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* Outer = Cast<UObject>(GetImplementingOuterObject(UAVVMOnlineInterface::StaticClass()));
	if (!ensureAlways(IsValid(Outer)))
	{
		UE_LOG(LogUI, Log, TEXT("Force Pull Player Profile Request. Failure! Outer doesn't Implement %s"), *UAVVMOnlineInterface::StaticClass()->GetName());
		return;
	}

	auto OnlineInterface = TScriptInterface<IAVVMOnlineInterface>(Outer);
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	IAVVMOnlineInterface::FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnForcePullPlayerProfileCompleted);

	UE_LOG(LogUI, Log, TEXT("Force Pull Player Profile Request. In-Progress..."));
	OnlineInterface->ForcePullPlayerProfile(Callback);
}

void UAVVMPlayerProfilePresenter::SetPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers additional data could be defined to properly log output here!
	UE_LOG(LogUI, Log, TEXT("Updating local Player Profile!"));

	auto* PlayerProfileViewModel = Cast<UAVVMPlayerProfileViewModel>(ViewModel.Get());
	if (IsValid(PlayerProfileViewModel))
	{
		PlayerProfileViewModel->SetPlayerProfile(Payload);
	}
}

void UAVVMPlayerProfilePresenter::StartPresenting()
{
	// TODO @gdemers we do not know if the Player Profile is a View that takes in all the screen
	// or simply a portion of the user HUD, etc... TBD by design for your project needs!
	// Most Importantly, we do not know if we should push :
	//	A) On a layer Stack
	//	B) On an extension Point
}

void UAVVMPlayerProfilePresenter::StopPresenting()
{
}

void UAVVMPlayerProfilePresenter::OnCommitPlayerProfileCompleted(const bool bWasSuccess,
                                                                        const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Committing Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update profile onSuccess
		SetPlayerProfile(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPlayerProfilePresenter::OnForcePullPlayerProfileCompleted(const bool bWasSuccess, const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Force Pull Player Profile Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update profile onSuccess
		SetPlayerProfile(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
