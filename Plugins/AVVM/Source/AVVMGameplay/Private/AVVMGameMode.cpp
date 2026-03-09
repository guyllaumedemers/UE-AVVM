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
#include "AVVMGameMode.h"

#include "AVVMGameplayModule.h"
#include "AVVMGameplayUtils.h"
#include "AVVMLogger.h"
#include "AVVMWorldSetting.h"
#include "TimerManager.h"
#include "Engine/NetConnection.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Rules/AVVMPlayerRule.h"
#include "Rules/AVVMSpawnPointRule.h"

AAVVMGameMode::AAVVMGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickBatching = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	SetReplicateMovement(false);
	bReplicates = false;

	// @gdemers disable inactive feature as this cause weird behaviour.
	InactivePlayerStateLifeSpan = 0;
	MaxInactivePlayers = 0;
}

void AAVVMGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GameStateSetEvent.AddUObject(this, &AAVVMGameMode::OnGameStateSet);
		OnGameStateSet(GameState);
	}
}

void AAVVMGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GameStateSetEvent.RemoveAll(this);
	}
}

void AAVVMGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasMatchEnded())
	{
		Terminate();
	}
}

bool AAVVMGameMode::IsMatchInProgress() const
{
	bool bIsInProgress = Super::IsMatchInProgress();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchProgressTag);
		bIsInProgress &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bIsInProgress);
	}

	return bIsInProgress;
}

bool AAVVMGameMode::ReadyToStartMatch_Implementation()
{
	bool bHasStarted = Super::ReadyToStartMatch_Implementation();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchStartTag);
		bHasStarted &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bHasStarted);
	}

	return bHasStarted;
}

bool AAVVMGameMode::ReadyToEndMatch_Implementation()
{
	bool bHasEnded = Super::ReadyToEndMatch_Implementation();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchEndTag);
		bHasEnded |= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bHasEnded);
	}

	return bHasEnded;
}

bool AAVVMGameMode::HasMatchEnded() const
{
	bool bHasEnded = Super::HasMatchEnded();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchPostEndTag);
		bHasEnded &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bHasEnded);
	}

	return bHasEnded;
}

void AAVVMGameMode::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (IsValid(NewGameState))
	{
		WorldSetting = Cast<AAVVMWorldSetting>(NewGameState->GetWorldSettings());
	}
}

void AAVVMGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// @gdemers Run validation on players that are joining a AGameSession, and kick out
	// whomever isn't authorized to join this session.
	bool bUnexpectedPlayer = false;
	if (WorldSetting.IsValid())
	{
		const auto* PlayerAcceptanceRule = Cast<UAVVMPlayerRule>(WorldSetting->GetRule(RuleTagAggregator.PlayerAcceptanceTag));
		bUnexpectedPlayer |= (IsValid(PlayerAcceptanceRule) ? PlayerAcceptanceRule->Predicate_UniqueNetId(UniqueId) : false);
	}

	if (!bUnexpectedPlayer)
	{
		return;
	}

	if (!IsValid(GameState))
	{
		return;
	}

	const auto* SearchResult = GameState->PlayerArray.FindByPredicate([Compare = UniqueId](const APlayerState* NewPlayerState)
	{
		return IsValid(NewPlayerState) && (Compare == NewPlayerState->GetUniqueId());
	});

	if (SearchResult != nullptr)
	{
		const APlayerController* PC = (*SearchResult)->GetPlayerController();
		if (!IsValid(PC))
		{
			return;
		}

		UNetConnection* NewConnection = PC->GetNetConnection();
		if (IsValid(NewConnection))
		{
			NewConnection->Close();
		}
	}
}

void AAVVMGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// @gdemers AGameMode::Login will assign a valid UniqueNetId to the APlayerState which means that it is safe to
	// enforce data refresh on the server after this steps.
	// example : any system handling caching of players using unique net id. 
}

void AAVVMGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	// TODO @gdemers Serialize Server sided information about the player leaving to
	// the project specific backend, or local save file, or drop progression if forfeiting.
	
	// IMPORTANT APlayerState is safe for access as the invocation call is made before we destroy
	// the player.
}

AActor* AAVVMGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	// @gdemers internal context object for tracking player attempts
	// to finding a valid start position.
	struct FAVVMPlayerRetryTracker
	{
		TMap<TWeakObjectPtr<AController>/*Player*/, int32/*Counter*/> CounterPerPlayer;
	};

	static FAVVMPlayerRetryTracker PlayerRetryTracker;
	int32& OutRetryCounter = PlayerRetryTracker.CounterPerPlayer.FindOrAdd(Player);

	if (!WorldSetting.IsValid())
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	const auto* SpawnPointRule = Cast<UAVVMSpawnPointRule>(WorldSetting->GetRule(RuleTagAggregator.SpawnPointSelectionTag));
	if (!IsValid(SpawnPointRule))
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	AActor* SpawnPoint = nullptr;
	bool bResult = false;

	TArray<AActor*> OutSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), OutSpawnPoints);

	bResult |= SpawnPointRule->Predicate_GetSpawnPoint(OutSpawnPoints, SpawnPoint);
	if (bResult)
	{
		// @gdemers safe reset counter upon finding valid spot.
		OutRetryCounter = 0;
		return SpawnPoint;
	}

	const bool bShouldClearPrevious = (false == SpawnPointRule->CanUsePreviousStartPositionOnFailure());
	if (bShouldClearPrevious)
	{
		Player->StartSpot.Reset();
	}

	const bool bCanDeferRetry = SpawnPointRule->CanRetrySearch();
	if (!ensureAlwaysMsgf(bCanDeferRetry,
	                      TEXT("Couldn't find a valid APlayerStart for %s. Retrying isn't supported! We are about to close the player net connection."),
	                      *GetNameSafe(Player)) ||
		!ensureAlwaysMsgf(OutRetryCounter < SpawnPointRule->GetMaxNumRetry(),
		                  TEXT("Couldn't find a valid APlayerStart for %s. We have exceeded the allowed number of retry (%d). We are about to close the player net connection."),
		                  *GetNameSafe(Player),
		                  SpawnPointRule->GetMaxNumRetry()))
	{
		UNetConnection* PlayerConnection = IsValid(Player) ? Player->GetNetConnection() : nullptr;
		if (IsValid(PlayerConnection))
		{
			PlayerConnection->Close();
		}

		return nullptr;
	}

	const auto OnStartPosition = [](TWeakObjectPtr<AAVVMGameMode> GameMode,
	                                TWeakObjectPtr<AController> RestartPlayer)
	{
		if (GameMode.IsValid())
		{
			GameMode->RestartPlayer(RestartPlayer.Get());
		}
	};

	FTimerHandle OutHandle;
	GetWorldTimerManager().SetTimer(OutHandle,
	                                FTimerDelegate::CreateWeakLambda(this, OnStartPosition, this, Player),
	                                SpawnPointRule->GetRetryRate(),
	                                false);

	// @gdemers increment counter so we can eventually timeout on failure.
	OutRetryCounter += 1;
	return nullptr;
}

void AAVVMGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	AVVM_LOGGER_ERROR(LogGameplay,
	                  this,
	                  NewPlayer,
	                  TEXT("Couldn't spawn a valid Pawn."));
	
	UNetConnection* PlayerConnection = IsValid(NewPlayer) ? NewPlayer->GetNetConnection() : nullptr;
	if (IsValid(PlayerConnection))
	{
		PlayerConnection->Close();
	}
}

void AAVVMGameMode::Terminate()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		TerminateDedicatedServer();
	}
	else
	{
		TerminateListenServer();
	}
}

void AAVVMGameMode::TerminateDedicatedServer()
{
	if (bAllowServerProcessExit)
	{
		// @gdemers By Killing the server process, the Engine::Tick will trigger a ClientTravel for
		// all players back to the Default map defined in your .ini file.
		FPlatformMisc::RequestExit(true);
	}
	else
	{
		RecycleProcess();
	}
}

void AAVVMGameMode::TerminateListenServer()
{
	RecycleProcess();
}

void AAVVMGameMode::RecycleProcess()
{
	if (IsValid(GameSession))
	{
		GameSession->ReturnToMainMenuHost();
	}
}
