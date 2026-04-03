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
#include "AVVMGameState.h"
#include "AVVMLogger.h"
#include "AVVMWorldSetting.h"
#include "TimerManager.h"
#include "Engine/NetConnection.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Rules/AVVMDefaultPawnSpawnRule.h"
#include "Rules/AVVMPlayerRule.h"
#include "Rules/AVVMSpawnPointRule.h"

FOnPlayerReadyForUnregistrationDelegate AAVVMGameMode::OnPlayerReadyForUnregistration;
FOnPlayerReadyForRegistrationDelegate AAVVMGameMode::OnPlayerReadyForRegistration;

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

FOnPlayerReadyForUnregistrationDelegate& AAVVMGameMode::GetOnPlayerReadyForUnregistration()
{
	return OnPlayerReadyForUnregistration;
}

FOnPlayerReadyForRegistrationDelegate& AAVVMGameMode::GetOnPlayerReadyForRegistration()
{
	return OnPlayerReadyForRegistration;
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
	bool bIsExpectedPlayer = false;
	if (ensureAlwaysMsgf(WorldSetting.IsValid(),
	                     TEXT("Invalid %s"),
	                     *GetNameSafe(AAVVMWorldSetting::StaticClass())))
	{
		const auto* PlayerAcceptanceRule = Cast<UAVVMPlayerAcceptanceRule>(WorldSetting->GetRule(RuleTagAggregator.PlayerAcceptanceTag));
		bIsExpectedPlayer = (IsValid(PlayerAcceptanceRule) ? PlayerAcceptanceRule->Predicate_IsExpectedUniqueNetId(UniqueId) : bDoesAcceptAnyPlayerLogin);
	}

	if (bIsExpectedPlayer)
	{
		return;
	}

	auto* NewGameState = Cast<AAVVMGameState>(GameState);
	if (IsValid(NewGameState))
	{
		NewGameState->RejectPlayerUniqueNetId(UniqueId);
	}
}

void AAVVMGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// @gdemers AGameMode::Login will assign a valid UniqueNetId to the APlayerState which means that it is safe to
	// enforce data refresh on the server after this steps.
	// example : any system handling caching of players using unique net id.
	RegisterPlayerWithAuthoritativeSubsystem(IsValid(NewPlayer) ? NewPlayer->GetPlayerState<APlayerState>() : nullptr);
}

void AAVVMGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// @gdemers IMPORTANT APlayerState is safe for access as the invocation call is made before we destroy the player.
	UnregisterPlayerFromAuthoritativeSubsystem(IsValid(Exiting) ? Exiting->GetPlayerState<APlayerState>() : nullptr);
}

void AAVVMGameMode::RegisterPlayerWithAuthoritativeSubsystem(const APlayerState* NewPlayer) const
{
	if (!ensureAlwaysMsgf(IsValid(NewPlayer), TEXT("Attempting to Register with an invalid PlayerState.")))
	{
		return;
	}

	const FUniqueNetIdPtr UniqueNetIdPtr = NewPlayer->GetUniqueId().GetV1();
	if (ensureAlwaysMsgf(UniqueNetIdPtr.IsValid(), TEXT("Attempting to Register with an invalid UniqueNetId.")))
	{
		// @gdemers IMPORTANT UAVVMNotificationSubsystem cannot be used here as we are targeting Subsystems and not actors.
		GetOnPlayerReadyForRegistration().Broadcast(*UniqueNetIdPtr, NewPlayer);
	}
}

void AAVVMGameMode::UnregisterPlayerFromAuthoritativeSubsystem(const APlayerState* NewPlayer) const
{
	if (!ensureAlwaysMsgf(IsValid(NewPlayer), TEXT("Attempting to Unregister with an invalid PlayerState.")))
	{
		return;
	}

	const FUniqueNetIdPtr UniqueNetIdPtr = NewPlayer->GetUniqueId().GetV1();
	if (ensureAlwaysMsgf(UniqueNetIdPtr.IsValid(), TEXT("Attempting to Unregister with an invalid UniqueNetId.")))
	{
		// @gdemers IMPORTANT UAVVMNotificationSubsystem cannot be used here as we are targeting Subsystems and not actors.
		GetOnPlayerReadyForUnregistration().Broadcast(*UniqueNetIdPtr, NewPlayer);
	}
}

AActor* AAVVMGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (!bShouldUseCustomPlayerStartPositionFilters)
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}
	
	// @gdemers internal context object for tracking player attempts
	// to finding a valid start position.
	struct FAVVMPlayerRetryTracker
	{
		TMap<TWeakObjectPtr<AController>/*Player*/, int32/*Counter*/> CounterPerPlayer;
	};

	static FAVVMPlayerRetryTracker PlayerRetryTracker;
	int32& OutRetryCounter = PlayerRetryTracker.CounterPerPlayer.FindOrAdd(Player);

	if (!ensureAlwaysMsgf(WorldSetting.IsValid(),
	                      TEXT("Invalid %s."),
	                      *GetNameSafe(AAVVMWorldSetting::StaticClass())))
	{
		return nullptr;
	}

	const auto* SpawnPointRule = Cast<UAVVMSpawnPointRule>(WorldSetting->GetRule(RuleTagAggregator.SpawnPointSelectionTag));
	if (!ensureAlwaysMsgf(IsValid(SpawnPointRule),
	                      TEXT("Invalid %s for tag %s."),
	                      *GetNameSafe(UAVVMSpawnPointRule::StaticClass()),
	                      *RuleTagAggregator.SpawnPointSelectionTag.ToString()))
	{
		return nullptr;
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

	const bool bCanUsePreviousStartPositionOnFailure = SpawnPointRule->CanUsePreviousStartPositionOnFailure();
	if (bCanUsePreviousStartPositionOnFailure && Player->StartSpot.IsValid())
	{
		return Player->StartSpot.Get();
	}
	else
	{
		// @gdemers returning nullptr will have AGameModeBase::RestartPlayer attempt assigning our
		// cached value as the default spawn position. we want to prevent such case!
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
			OutRetryCounter = 0;
			PlayerConnection->Close();
		}

		return nullptr;
	}

	const auto OnStartPosition = [](const TWeakObjectPtr<AAVVMGameMode>& GameMode,
	                                const TWeakObjectPtr<AController>& RestartPlayer)
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

APawn* AAVVMGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	if (!bShouldDeferDefaultPawnCreation)
	{
		return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	}

	// @gdemers internal context object for tracking player attempts
	// to spawning the default pawn tie to this PC.
	struct FAVVMPlayerRetryTracker
	{
		TMap<TWeakObjectPtr<AController>/*Player*/, int32/*Counter*/> CounterPerPlayer;
	};

	static FAVVMPlayerRetryTracker PlayerRetryTracker;
	int32& OutRetryCounter = PlayerRetryTracker.CounterPerPlayer.FindOrAdd(NewPlayer);

	// @gdemers we expect users to implement their own spawn process based on project defined rules.
	if (!ensureAlwaysMsgf(WorldSetting.IsValid(),
	                      TEXT("Invalid %s."),
	                      *GetNameSafe(AAVVMWorldSetting::StaticClass())))
	{
		return nullptr;
	}

	const auto* DefaultPawnSpawnRule = Cast<UAVVMDefaultPawnSpawnRule>(WorldSetting->GetRule(RuleTagAggregator.DefaultPawnSpawnConditionsTag));
	if (!ensureAlwaysMsgf(IsValid(DefaultPawnSpawnRule),
	                      TEXT("Invalid %s for tag %s."),
	                      *GetNameSafe(UAVVMDefaultPawnSpawnRule::StaticClass()),
	                      *RuleTagAggregator.DefaultPawnSpawnConditionsTag.ToString()))
	{
		return nullptr;
	}

	const bool bHasMetRequirements = DefaultPawnSpawnRule->Predicate_HasMetDefaultPawnSpawnRequirements(this, Cast<APlayerController>(NewPlayer));
	if (bHasMetRequirements)
	{
		// @gdemers spawn the default pawn as normal. we have met all conditions, and are ready to play safely.
		return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	}

	if (!ensureAlwaysMsgf(OutRetryCounter < DefaultPawnSpawnRule->GetMaxNumRetry(),
	                      TEXT("Couldn't spawn the default pawn for %s. We have exceeded the allowed number of retry (%d). We are about to close the player net connection."),
	                      *GetNameSafe(NewPlayer),
	                      DefaultPawnSpawnRule->GetMaxNumRetry()))
	{
		UNetConnection* PlayerConnection = IsValid(NewPlayer) ? NewPlayer->GetNetConnection() : nullptr;
		if (IsValid(PlayerConnection))
		{
			OutRetryCounter = 0;
			PlayerConnection->Close();
		}

		return nullptr;
	}

	const auto OnRestartPlayerDeferred = [](const TWeakObjectPtr<AAVVMGameMode>& GameMode,
	                                        const TWeakObjectPtr<AController>& RestartPlayer)
	{
		if (GameMode.IsValid())
		{
			GameMode->RestartPlayer(RestartPlayer.Get());
		}
	};

	// @gdemers defer request until we have met all proper requirements.
	FTimerHandle OutHandle;
	GetWorldTimerManager().SetTimer(OutHandle,
	                                FTimerDelegate::CreateWeakLambda(this, OnRestartPlayerDeferred, this, NewPlayer),
	                                DefaultPawnSpawnRule->GetRetryRate(),
	                                false);

	// @gdemers increment counter so we can eventually timeout on failure.
	OutRetryCounter += 1;
	return nullptr;
}

void AAVVMGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	// @gdemers we only allow failure in the case where our project specific impl doesn't allow retries. Connection
	// closure will be handled by the deferred case, where our attempt to multiple retries has timed out.
	if (bShouldDeferDefaultPawnCreation)
	{
		return;
	}
	
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
