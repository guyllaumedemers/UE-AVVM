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

#include "AVVM.h"
#include "AVVMGameMode.h"
#include "Party/AVVMPlayerManagerViewModel.h"

AActor* UAVVMPlayerManagerPresenter::GetOuterKey() const
{
	return GetTypedOuter<AAVVMGameMode>();
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RefreshAllPlayers(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("[Local] Refreshing all Players Visual representation!"));
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RemoteConnectNewPlayer(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers call to external system which create a static representation of a player or does something similar.
	BP_AddNewPlayerConnection(Payload);
	UE_LOG(LogUI, Log, TEXT("[Remote] Adding new Player Connection!"));
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RemoteDisconnectPlayer(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers call to external system which destroy a static representation of a player or does something similar.
	BP_RemoveNewPlayerConnection(Payload);
	UE_LOG(LogUI, Log, TEXT("[Remote] Removing existing Player Connection!"));
	SetPlayerConnections(Payload);
}

void UAVVMPlayerManagerPresenter::SetPlayerConnections(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Updating Player Connections!"));

	auto* HostConfigurationViewModel = Cast<UAVVMPlayerManagerViewModel>(ViewModel.Get());
	if (IsValid(HostConfigurationViewModel))
	{
		HostConfigurationViewModel->SetPlayerConnections(Payload);
	}
}

void UAVVMPlayerManagerPresenter::StartPresenting()
{
	// TODO @gdemers we do not know if the PartyManager is a View that takes in all the screen
	// or simply a portion of the user HUD, etc... TBD by design for your project needs!
	// Most Importantly, we do not know if we should push :
	//	A) On a layer Stack
	//	B) On an extension Point
}

void UAVVMPlayerManagerPresenter::StopPresenting()
{
}
