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
#include "Player/AVVMPlayerConfigurationPresenter.h"

#include "AVVM.h"
#include "AVVMPlayerState.h"
#include "Player/AVVMPlayerConfigurationViewModel.h"

AActor* UAVVMPlayerConfigurationPresenter::GetOuterKey() const
{
	return GetTypedOuter<AAVVMPlayerState>();
}

void UAVVMPlayerConfigurationPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMPlayerConfigurationPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMPlayerConfigurationPresenter::BP_OnNotificationReceived_RemotePlayerLevelUp(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers made this call separate to Changed event call so we can run the edge case of only leveling once if during the
	// session the Outer is destroyed and re-created!
	const auto* RemoteConnection = Payload.GetPtr<FAVVMPlayerConnection>();
	if (ensure(RemoteConnection != nullptr) && DoesMatchPlayerConnection(*RemoteConnection))
	{
		// @gdemers here our notification channel are type specific and not instance specific
		// we expect some validation to be run before executing any code here as ALL players configuration will try
		// executing the following code.
		UE_LOG(LogUI, Log, TEXT("[Remote] Has Level Up!"));
		// @gdemers trigger emote event for separate system which handle showing above head animation like league of legend does.
		BP_EmotePlayer(Payload);
		SetPlayerConnection(Payload);
	}
}

void UAVVMPlayerConfigurationPresenter::BP_OnNotificationReceived_RemotePlayerProfileChanged(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	const auto* RemoteConnection = Payload.GetPtr<FAVVMPlayerConnection>();
	if (ensure(RemoteConnection != nullptr) && DoesMatchPlayerConnection(*RemoteConnection))
	{
		UE_LOG(LogUI, Log, TEXT("[Remote] Player Profile Changed!"));
		// @gdemers trigger equipping event for any external system which should receive the modified entry/equipped item that changed.
		BP_StripNakedPlayer(Payload);
		SetPlayerConnection(Payload);
	}
}

void UAVVMPlayerConfigurationPresenter::SetPlayerConnection(const TInstancedStruct<FAVVMNotificationPayload>& Payload) const
{
	UE_LOG(LogUI, Log, TEXT("[Remote] Refreshing Player Profile!"));

	auto* PlayerConfigurationViewModel = Cast<UAVVMPlayerConfigurationViewModel>(ViewModel.Get());
	if (IsValid(PlayerConfigurationViewModel))
	{
		PlayerConfigurationViewModel->SetPlayerConnection(Payload);
	}
}

bool UAVVMPlayerConfigurationPresenter::DoesMatchPlayerConnection(const FAVVMPlayerConnection& RemoteConnection) const
{
	auto* PlayerConfigurationViewModel = Cast<UAVVMPlayerConfigurationViewModel>(ViewModel.Get());
	if (IsValid(PlayerConfigurationViewModel))
	{
		return PlayerConfigurationViewModel->DoesMatchPlayerConnection(RemoteConnection);
	}

	return false;
}

void UAVVMPlayerConfigurationPresenter::StartPresenting()
{
	// TODO @gdemers we do not know if the PartyManager is a View that takes in all the screen
	// or simply a portion of the user HUD, etc... TBD by design for your project needs!
	// Most Importantly, we do not know if we should push :
	//	A) On a layer Stack
	//	B) On an extension Point
}

void UAVVMPlayerConfigurationPresenter::StopPresenting()
{
}
