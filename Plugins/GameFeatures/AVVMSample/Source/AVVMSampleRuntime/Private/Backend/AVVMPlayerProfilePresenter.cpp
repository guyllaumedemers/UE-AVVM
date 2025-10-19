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
#include "AVVMOnlineInterfaceUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "Backend/AVVMPlayerProfileViewModel.h"
#include "GameFramework/GameMode.h"

AActor* UAVVMPlayerProfilePresenter::GetOuterKey() const
{
	return GetTypedOuter<AGameMode>();
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
	TScriptInterface<IAVVMOnlineIdentityInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineIdentityInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnCommitPlayerProfileCompleted);

	// @gdemers we obviously wouldnt commit an empty player account. we expect the online subsystem to run validation on the account
	// before executing any request to overwrite the backend data.
	const auto* ModifiedPlayerProfile = Payload.GetPtr<FAVVMPlayerProfileProxy>();
	if (ModifiedPlayerProfile != nullptr)
	{
		UE_LOG(LogAVVMOnline, Log, TEXT("Committing Player Profile Request. In-Progress..."));
		OnlineInterface->CommitModifiedPlayerProfile(*ModifiedPlayerProfile, Callback);
	}
	else
	{
		UE_LOG(LogAVVMOnline, Log, TEXT("Committing Empty Player Profile Request. In-Progress..."));
		OnlineInterface->CommitModifiedPlayerProfile(FAVVMPlayerProfileProxy{}, Callback);
	}
}

void UAVVMPlayerProfilePresenter::BP_OnNotificationReceived_ForcePullPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineIdentityInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineIdentityInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnForcePullPlayerProfileCompleted);

	UE_LOG(LogAVVMOnline, Log, TEXT("Force Pull Player Profile Request. In-Progress..."));
	OnlineInterface->ForcePullPlayerProfile(Callback);
}

void UAVVMPlayerProfilePresenter::BP_OnNotificationReceived_ProcessPlayerRequest(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	const auto* PlayerRequest = Payload.GetPtr<FAVVMPlayerRequest>();
	if (!ensure(PlayerRequest != nullptr))
	{
		return;
	}

	const FString EventTypeString(EnumToString(PlayerRequest->RequestType));
	UE_LOG(LogAVVMOnline,
	       Log,
	       TEXT("Processing Player Request. Type: %s, In-Progress..."),
	       *EventTypeString);

	const bool bShouldTrade = (PlayerRequest->RequestType == EAVVMPlayerRequestType::Trade);
	if (bShouldTrade)
	{
		TryTrade(*PlayerRequest);
		return;
	}

	const bool bShouldAddFriend = (PlayerRequest->RequestType == EAVVMPlayerRequestType::AddFriend);
	if (bShouldAddFriend)
	{
		TryAddFriend(*PlayerRequest);
		return;
	}

	const bool bShouldRemoveFriend = (PlayerRequest->RequestType == EAVVMPlayerRequestType::RemoveFriend);
	if (bShouldRemoveFriend)
	{
		TryRemoveFriend(*PlayerRequest);
		return;
	}

	const bool bShouldInvitePlayer = (PlayerRequest->RequestType == EAVVMPlayerRequestType::InvitePlayer);
	if (bShouldInvitePlayer)
	{
		TryInvitePlayer(*PlayerRequest);
		return;
	}
}

void UAVVMPlayerProfilePresenter::SetPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers additional data could be defined to properly log output here!
	UE_LOG(LogAVVMOnline, Log, TEXT("Updating local Player Profile!"));

	auto* PlayerProfileViewModel = Cast<UAVVMPlayerProfileViewModel>(ViewModel.Get());
	if (IsValid(PlayerProfileViewModel))
	{
		PlayerProfileViewModel->SetPlayerProfile(Payload);
	}
}

void UAVVMPlayerProfilePresenter::StartPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(this);
	if (!ensureAlwaysMsgf(IsValid(LocalPlayer),
	                      TEXT("UAVVMPlayerProfilePresenter couldn't find a valid LocalPlayer!")))
	{
		return;
	}

	FAVVMPrimaryGameLayoutContextArgs ContextArgs;
	ContextArgs.LayerTag = TargetTag;
	ContextArgs.WidgetClass = WidgetClass;
	PushContentToPrimaryGameLayout(this, LocalPlayer, ContextArgs);
}

void UAVVMPlayerProfilePresenter::StopPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(this);
	if (IsValid(LocalPlayer))
	{
		PopContentFromPrimaryGameLayout(LocalPlayer, ActivatableView.Get());
	}
}

void UAVVMPlayerProfilePresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}

void UAVVMPlayerProfilePresenter::OnCommitPlayerProfileCompleted(const bool bWasSuccess,
                                                                 const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Committing Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
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
	UE_LOG(LogAVVMOnline, Log, TEXT("Force Pull Player Profile Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
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

void UAVVMPlayerProfilePresenter::TryInvitePlayer(const FAVVMPlayerRequest& PlayerRequest)
{
	TScriptInterface<IAVVMOnlineFriendInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineFriendInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnInvitePlayerCompleted);
	UE_LOG(LogAVVMOnline, Log, TEXT("Invite Player Request. In-Progress..."));

	OnlineInterface->InvitePlayer(PlayerRequest, Callback);
}

void UAVVMPlayerProfilePresenter::OnInvitePlayerCompleted(const bool bWasSuccess,
                                                          const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Invite Player Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers handle notification to the friend presenter system to update any visual representation of the friend list
		// imply that we aren't using steam service and have a custom screen in the game for displaying this information.
		BP_InvitePlayer(Payload);

		// @gdemers Post-InvitePlayer, we expect the backend to execute the following calls :
		//		A) Execute the callback from the caller that requested the invite operation
		//		B) Notify all Party members of a new Player Connection (i.e UAVVMPlayerManagerPresenter::BP_OnNotificationReceived_RemoteConnectNewPlayer)
		//		C) Execute a new Join Party for the invited player which will reinit it's visual state.

		// @gdemers OnSucess, nothing should happen here! the above statement will be handled from a backend party update call which
		// will refresh the content of the party.
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPlayerProfilePresenter::TryBlockPlayer(const FAVVMPlayerRequest& PlayerRequest)
{
	TScriptInterface<IAVVMOnlineFriendInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineFriendInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnBlockPlayerCompleted);
	UE_LOG(LogAVVMOnline, Log, TEXT("Block Player Request. In-Progress..."));

	OnlineInterface->BlockPlayer(PlayerRequest, Callback);
}

void UAVVMPlayerProfilePresenter::OnBlockPlayerCompleted(const bool bWasSuccess,
                                                         const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Block Request Add Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers handle notification to the friend presenter system to update any visual representation of the friend list
		// imply that we aren't using steam service and have a custom screen in the game for displaying this information.
		BP_BlockPlayer(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPlayerProfilePresenter::TryAddFriend(const FAVVMPlayerRequest& PlayerRequest)
{
	TScriptInterface<IAVVMOnlineFriendInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineFriendInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnAddFriendCompleted);
	UE_LOG(LogAVVMOnline, Log, TEXT("Adding new Friend Request. In-Progress..."));

	OnlineInterface->AddFriend(PlayerRequest, Callback);
}

void UAVVMPlayerProfilePresenter::OnAddFriendCompleted(const bool bWasSuccess,
                                                       const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	// @gdemers note that friend request are pending until accepted by the receiving end of the request.
	UE_LOG(LogAVVMOnline, Log, TEXT("Friend Request Add Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers handle notification to the friend presenter system to update any visual representation of the friend list
		// imply that we aren't using steam service and have a custom screen in the game for displaying this information.
		BP_AddFriend(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPlayerProfilePresenter::TryRemoveFriend(const FAVVMPlayerRequest& PlayerRequest)
{
	TScriptInterface<IAVVMOnlineFriendInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineFriendInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnRemoveFriendCompleted);
	UE_LOG(LogAVVMOnline, Log, TEXT("Removing Friend Request. In-Progress..."));

	OnlineInterface->RemoveFriend(PlayerRequest, Callback);
}

void UAVVMPlayerProfilePresenter::OnRemoveFriendCompleted(const bool bWasSuccess,
                                                          const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Friend Request Remove Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers handle notification to the friend presenter system to update any visual representation of the friend list
		// imply that we aren't using steam service and have a custom screen in the game for displaying this information.
		BP_RemoveFriend(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMPlayerProfilePresenter::TryTrade(const FAVVMPlayerRequest& PlayerRequest)
{
	TScriptInterface<IAVVMOnlineIdentityInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineIdentityInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMPlayerProfilePresenter::OnTradeCompleted);
	UE_LOG(LogAVVMOnline, Log, TEXT("Trade Request. In-Progress..."));

	OnlineInterface->Trade(PlayerRequest, Callback);
}

void UAVVMPlayerProfilePresenter::OnTradeCompleted(const bool bWasSuccess,
                                                   const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogAVVMOnline, Log, TEXT("Trade Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
