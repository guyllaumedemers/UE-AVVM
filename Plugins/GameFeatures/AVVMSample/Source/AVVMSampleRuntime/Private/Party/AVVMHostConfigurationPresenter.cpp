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
#include "Party/AVVMHostConfigurationPresenter.h"

#include "AVVM.h"
#include "AVVMGameMode.h"
#include "Backend/AVVMOnlineInterfaceUtils.h"
#include "Party/AVVMHostConfigurationViewModel.h"

AActor* UAVVMHostConfigurationPresenter::GetOuterKey() const
{
	return GetTypedOuter<AAVVMGameMode>();
}

void UAVVMHostConfigurationPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMHostConfigurationPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMHostConfigurationPresenter::BP_OnNotificationReceived_CommitModifiedHostConfiguration(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	const bool bIsFirstPlayerHosting = UAVVMOnlineInterfaceUtils::IsFirstPlayerHosting(this, OnlineInterface);
	if (!bIsFirstPlayerHosting)
	{
		UE_LOG(LogUI, Log, TEXT("Commit Modified Host Configuration Request. Failure! Only the Owner of the Party can update the Host configuration!"));
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMHostConfigurationPresenter::OnCommitModifiedHostConfigurationCompleted);

	const auto* ModifiedHostConfiguration = Payload.GetPtr<FAVVMHostConfiguration>();
	if (ModifiedHostConfiguration != nullptr)
	{
		UE_LOG(LogUI, Log, TEXT("Commit Modified Host Configuration Request. In-Progress..."));
		OnlineInterface->CommitModifiedHostConfiguration(*ModifiedHostConfiguration, Callback);
	}
	else
	{
		UE_LOG(LogUI, Log, TEXT("Commit Empty Host Configuration Request. In-Progress..."));
		OnlineInterface->CommitModifiedHostConfiguration(FAVVMHostConfiguration{}, Callback);
	}
}

void UAVVMHostConfigurationPresenter::BP_OnNotificationReceived_ForcePullHostConfiguration(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMHostConfigurationPresenter::OnForcePullHostConfigurationCompleted);

	UE_LOG(LogUI, Log, TEXT("Force Pull Host Configuration Request. In-Progress..."));
	OnlineInterface->ForcePullHostConfiguration(Callback);
}

void UAVVMHostConfigurationPresenter::BP_OnNotificationReceived_RemoteRefreshHostConfiguration(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Remote Refresh Host Configuration Request. Execute..."));
	SetHostConfiguration(Payload);
}

void UAVVMHostConfigurationPresenter::SetHostConfiguration(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers additional data could be defined to properly log output here!
	UE_LOG(LogUI, Log, TEXT("Updating local Host Configuration!"));

	auto* HostConfigurationViewModel = Cast<UAVVMHostConfigurationViewModel>(ViewModel.Get());
	if (IsValid(HostConfigurationViewModel))
	{
		HostConfigurationViewModel->SetHostConfiguration(Payload);
	}
}

void UAVVMHostConfigurationPresenter::StartPresenting()
{
	// TODO @gdemers we do not know if the PartyManager is a View that takes in all the screen
	// or simply a portion of the user HUD, etc... TBD by design for your project needs!
	// Most Importantly, we do not know if we should push :
	//	A) On a layer Stack
	//	B) On an extension Point
}

void UAVVMHostConfigurationPresenter::StopPresenting()
{
}

void UAVVMHostConfigurationPresenter::OnCommitModifiedHostConfigurationCompleted(const bool bWasSuccess,
                                                                                 const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Commit Modified Host Configuration Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update host configuration onSuccess
		SetHostConfiguration(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMHostConfigurationPresenter::OnForcePullHostConfigurationCompleted(const bool bWasSuccess,
                                                                            const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Force Pull Host Configuration Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update host configuration onSuccess
		SetHostConfiguration(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
