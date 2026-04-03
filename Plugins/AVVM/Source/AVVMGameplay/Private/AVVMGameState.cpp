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
#include "AVVMGameState.h"

#include "AVVMLogger.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMPlayerState.h"
#include "Engine/NetConnection.h"
#include "Net/UnrealNetwork.h"
#include "Tags/AVVMGameplayTags.h"

AAVVMGameState::AAVVMGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickBatching = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	SetReplicateMovement(false);
	bReplicates = true;
}

void AAVVMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAVVMGameState::AddPlayerState(APlayerState* PlayerState)
{
	// @gdemers IMPORTANT : player rejection cannot be handled here as we dont yet have a valid UniqueNetId.
	Super::AddPlayerState(PlayerState);

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_ONPLAYERSTATE_ADDED_OR_REMOVED;
	ContextArgs.Payload = FAVVMNotificationPayload::Make<FAVVMPlayerStatePayload>(PlayerState, true);
	ContextArgs.Target = this;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void AAVVMGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_ONPLAYERSTATE_ADDED_OR_REMOVED;
	ContextArgs.Payload = FAVVMNotificationPayload::Make<FAVVMPlayerStatePayload>(PlayerState, false);
	ContextArgs.Target = this;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void AAVVMGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_MATCH_WAITING;
	ContextArgs.Payload = FAVVMNotificationPayload::Empty;
	ContextArgs.Target = this;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void AAVVMGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_MATCH_STARTED;
	ContextArgs.Payload = FAVVMNotificationPayload::Empty;
	ContextArgs.Target = this;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void AAVVMGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_MATCH_ENDED;
	ContextArgs.Payload = FAVVMNotificationPayload::Empty;
	ContextArgs.Target = this;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void AAVVMGameState::HandleLeavingMap()
{
	Super::HandleLeavingMap();

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_MATCH_LEAVINGMAP;
	ContextArgs.Payload = FAVVMNotificationPayload::Empty;
	ContextArgs.Target = this;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void AAVVMGameState::RejectPlayerUniqueNetId(const FUniqueNetIdRepl& UniqueNetId)
{
	// @gdemers we cache our UniqueNetId as the AGameState::PlayerArray only initialize later.
	RejectedPlayers.Add(UniqueNetId.GetV1());
}

bool AAVVMGameState::Server_HandleRejectedLogin(APlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		return false;
	}

	const FUniqueNetIdPtr UniqueNetIdPtr = PlayerState->GetUniqueId().GetV1();
	if (!RejectedPlayers.Contains(UniqueNetIdPtr))
	{
		return false;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                GetTypedOuter<AActor>(),
	                this,
	                TEXT("Unexpected login request from %s. Rejecting!"),
	                *UniqueNetIdPtr->ToString());

	const APlayerController* PC = PlayerState->GetPlayerController();
	if (!ensureAlwaysMsgf(IsValid(PC),
	                      TEXT("Require a valid %s to reject a player."),
	                      *GetNameSafe(APlayerController::StaticClass())))
	{
		return false;
	}

	UNetConnection* NewConnection = PC->GetNetConnection();
	if (ensureAlwaysMsgf(IsValid(NewConnection),
	                     TEXT("Require a valid %s to reject a player."),
	                     *GetNameSafe(UNetConnection::StaticClass())))
	{
		NewConnection->Close();

		// @gdemers remove our id from the list for subsequent attempt to re-log to this session.
		// example : player wasn't initially expected, but a spot may be freed on retry.
		RejectedPlayers.Remove(UniqueNetIdPtr);
	}

	return true;
}

void AAVVMGameState::Multicast_NotifyPlayerStateRejection_Implementation(APlayerState* PlayerState)
{
	// @gdemers : PlayerArray called within Super::RemovePlayerState is simulated, not replicated
	// which require a RPC update.
	RemovePlayerState(PlayerState);
}
