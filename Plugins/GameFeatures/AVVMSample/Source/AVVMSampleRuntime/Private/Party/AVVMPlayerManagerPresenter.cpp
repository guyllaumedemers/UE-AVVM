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
#include "Party/AVVMPlayerManagerPresenter.h"

#include "AVVMOnline.h"
#include "AVVMUtils.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/GameMode.h"
#include "Party/AVVMPlayerManagerViewModel.h"

AActor* UAVVMPlayerManagerPresenter::GetOuterKey() const
{
	return GetTypedOuter<AGameMode>();
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();

	// @gdemers can happen post-join/post-exit (and invoked through BP_OnNotificationReceived_RefreshAllPlayers) which also imply
	// that we have just entered a new party.
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RefreshAllPlayers(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers event received post-join of a party and where we expect to initialize all player representation
	// and create player states which own a AVVMComponent/AVVMPlayerConfigurationPresenter.
	BP_RefreshPlayers(Payload);
	UE_LOG(LogAVVMOnline, Log, TEXT("[Local] Refreshing all Players Visual representation!"));
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RemoteConnectNewPlayer(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers call to external system which create a static representation of a player or does something similar.
	BP_ConnectPlayer(Payload);
	UE_LOG(LogAVVMOnline, Log, TEXT("[Remote] Adding new Player Connection!"));
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RemoteDisconnectPlayer(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers call to external system which destroy a static representation of a player or does something similar.
	BP_DisconnectPlayer(Payload);
	UE_LOG(LogAVVMOnline, Log, TEXT("[Remote] Removing existing Player Connection!"));
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::SetPlayerConnections(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Updating Player Connections!"));

	auto* PlayerManagerViewModel = Cast<UAVVMPlayerManagerViewModel>(ViewModel.Get());
	if (IsValid(PlayerManagerViewModel))
	{
		PlayerManagerViewModel->SetPlayerConnections(FAVVMOnlineModule::GetJsonParser(), Payload);
	}
}

void UAVVMPlayerManagerPresenter::StartPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtils::GetFirstOrTargetLocalPlayer(this);
	if (!ensureAlwaysMsgf(IsValid(LocalPlayer),
	                      TEXT("UAVVMPlayerManagerPresenter couldn't find a valid LocalPlayer!")))
	{
		return;
	}

	FAVVMPrimaryGameLayoutContextArgs ContextArgs;
	ContextArgs.LayerTag = TargetTag;
	ContextArgs.WidgetClass = WidgetClass;
	PushContentToPrimaryGameLayout(this, LocalPlayer, ContextArgs);
}

void UAVVMPlayerManagerPresenter::StopPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtils::GetFirstOrTargetLocalPlayer(this);
	if (IsValid(LocalPlayer))
	{
		PopContentFromPrimaryGameLayout(LocalPlayer, ActivatableView.Get());
	}
}

void UAVVMPlayerManagerPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtils::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}
