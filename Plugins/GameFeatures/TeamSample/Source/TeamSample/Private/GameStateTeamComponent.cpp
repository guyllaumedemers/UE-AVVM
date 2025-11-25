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

#include "GameStateTeamComponent.h"

#include "AVVMGameplayUtils.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMPlayerState.h"
#include "AVVMWorldSetting.h"
#include "NativeGameplayTags.h"
#include "TeamObject.h"
#include "TeamRule.h"
#include "TeamSample.h"
#include "TeamSettings.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_WORLD_RULE_TEAM, "WorldRule.Team");
UE_DEFINE_GAMEPLAY_TAG(TAG_TEAM_DESTROYED_NOTIFICATION, "TeamSample.Notification.TeamDestroyed");
UE_DEFINE_GAMEPLAY_TAG(TAG_TEAM_CREATED_NOTIFICATION, "TeamSample.Notification.TeamCreated");

UGameStateTeamComponent::UGameStateTeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);
	
	bReplicateUsingRegisteredSubObjectList = true;
}

void UGameStateTeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameStateTeamComponent, Teams);
}

void UGameStateTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	auto* Outer = GetTypedOuter<AGameState>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogTeamSample,
	       Log,
	       TEXT("Executed from \"%s\". Adding UGameStateTeamComponent to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	OwningOuter = Outer;

#if WITH_SERVER_CODE
	if (Outer->HasAuthority())
	{
		GetTeamRuleOnAuthority();

		FAVVMOnChannelNotifiedSingleCastDelegate Callback;
		Callback.BindDynamic(this, &UGameStateTeamComponent::OnPlayerStateAddedOrRemoved);

		FAVVMObserverContextArgs ContextArgs;
		ContextArgs.ChannelTag = PlayerStateChannelTag;
		ContextArgs.Callback = Callback;
		UAVVMNotificationSubsystem::Static_RegisterObserver(this, ContextArgs);

		for (const APlayerState* NewPlayerState : Outer->PlayerArray)
		{
			OnPlayerStateAdded(NewPlayerState);
		}
	}
#endif
}

void UGameStateTeamComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	StreamableHandle.Reset();

	for (auto Iterator = Teams.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	auto* Outer = Cast<AGameState>(OwningOuter.Get());
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogTeamSample,
	       Log,
	       TEXT("Executed from \"%s\". Removing UGameStateTeamComponent from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

#if WITH_SERVER_CODE
	if (Outer->HasAuthority())
	{
		FAVVMObserverContextArgs ContextArgs;
		ContextArgs.ChannelTag = PlayerStateChannelTag;
		UAVVMNotificationSubsystem::Static_UnregisterObserver(this, ContextArgs);
	}
#endif
}

void UGameStateTeamComponent::OnPlayerStateAddedOrRemoved(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
	const auto* Payload = NewPayload.GetPtr<FAVVMPlayerStatePayload>();
	if (!ensureAlwaysMsgf(Payload != nullptr,
	                      TEXT("Payload couldnt be casted to FAVVMPlayerStatePayload type")))
	{
		return;
	}

	const bool bWasAddedOrRemoved = Payload->bWasAddedOrRemoved;
	if (bWasAddedOrRemoved)
	{
		OnPlayerStateAdded(Payload->PlayerState.Get());
	}
	else
	{
		OnPlayerStateRemoved(Payload->PlayerState.Get());
	}
}

void UGameStateTeamComponent::OnPlayerStateAdded(const APlayerState* NewPlayerState)
{
	if (IsValid(NewPlayerState))
	{
		UE_LOG(LogTeamSample,
		       Log,
		       TEXT("Executed from \"%s\". OnPlayerStateAdded : \"%s\"."),
		       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData(),
		       *NewPlayerState->GetName());
	}
	
	// Problem : APlayerState is added by the game when a player attempt reaching the game, but
	// we may have already requested from the backend the parties composing the game.
	// As such, our pending state may have been modified during the time we wait for the request to respond,
	// and act on a modified collection which isnt accounted for in the party Context struct.

	const bool bIsLocked = SynchronizationLock.IsLocked();
	if (!bIsLocked)
	{
		PendingPlayerStates.Add(TWeakObjectPtr(NewPlayerState));
		// @gdemers theres two cases here to cover :
		// A) We have just entered a new game. we need to configure players team.
		// B) A player joined during an on-going game, and we need to configure their team.
		RequestTeams();
	}
	else
	{
		ScopedLockedPlayerStates.Add(TWeakObjectPtr(NewPlayerState));
	}
}

void UGameStateTeamComponent::OnPlayerStateRemoved(const APlayerState* NewPlayerState)
{
	if (IsValid(NewPlayerState))
	{
		UE_LOG(LogTeamSample,
		       Log,
		       TEXT("Executed from \"%s\". OnPlayerStateRemoved : \"%s\"."),
		       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData(),
		       *NewPlayerState->GetName());
	}
	
	// @gdemers theres only one case here to cover :
	// A) removal of a player who's already in game.
	UTeamObject* Team = UTeamUtils::UTeamUtils::FindTeam(Teams, NewPlayerState);
	if (IsValid(Team))
	{
		Team->UnRegisterPlayerState(NewPlayerState);
	}
}

void UGameStateTeamComponent::GetTeamRuleOnAuthority()
{
	const auto OnAsyncLoadComplete = [](const TWeakObjectPtr<UGameStateTeamComponent>& NewGameStateTeamComponent,
	                                    const TWeakObjectPtr<AAVVMWorldSetting>& NewWorldSettings)
	{
		if (!NewGameStateTeamComponent.IsValid() || !NewWorldSettings.IsValid())
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = NewGameStateTeamComponent->StreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		const auto* RuleClass = Cast<UClass>(Handle->GetLoadedAsset());
		if (!IsValid(RuleClass))
		{
			return;
		}

		NewGameStateTeamComponent->TeamRule = NewWorldSettings->GetOrCreatePluginRule<UTeamRule>(TAG_WORLD_RULE_TEAM, RuleClass);
		NewGameStateTeamComponent->RequestTeams();
	};

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (IsValid(WorldSettings))
	{
		const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncLoadComplete, TWeakObjectPtr(this), TWeakObjectPtr(WorldSettings));
		StreamableHandle = WorldSettings->AsyncLoadPluginRule(UTeamSettings::GetTeamRuleClass(), Callback);
	}
}

void UGameStateTeamComponent::RequestTeams()
{
	if (!TeamRule.IsValid())
	{
		return;
	}

	if (SynchronizationLock.IsLocked() || (PendingPlayerStates.IsEmpty() && ScopedLockedPlayerStates.IsEmpty()))
	{
		return;
	}

	if (!ScopedLockedPlayerStates.IsEmpty())
	{
		PendingPlayerStates.Append(ScopedLockedPlayerStates);
		ScopedLockedPlayerStates.Reset();
	}

	// @gdemers can be locked from our async loading Rule process and subsequent events from an incoming player
	// connections joining the game.
	SynchronizationLock.Lock();

	// @gdemers we expect your project to have some form of backend from which we can retrieve information
	// based on GameSession information.
	FOnBackendTeamRequestCompleteDelegate Callback;
	Callback.BindDynamic(this, &UGameStateTeamComponent::OnTeamReceived);
	GetBackendTeams(Callback);
}

void UGameStateTeamComponent::GetBackendTeams(const FOnBackendTeamRequestCompleteDelegate& Callback)
{
	const auto* GameState = Cast<AGameState>(OwningOuter.Get());
	if (!IsValid(GameState))
	{
		return;
	}

	const AGameModeBase* GameMode = GameState->AuthorityGameMode;
	if (!IsValid(GameMode))
	{
		return;
	}

	const AGameSession* GameSession = GameMode->GameSession;
	if (IsValid(GameSession))
	{
		BP_RequestGameSessionParties(GameSession->SessionName, Callback);
	}
}

void UGameStateTeamComponent::OnTeamReceived(const bool bWasSuccess,
                                             const TArray<FAVVMPartyProxy>& NewParties)
{
	const auto OutOfScopeCallback = [](const TWeakObjectPtr<UGameStateTeamComponent>& Caller)
	{
		if (Caller.IsValid())
		{
			Caller->RequestTeams();
		}
	};

	const auto Callback = FSimpleDelegate::CreateWeakLambda(this, OutOfScopeCallback, TWeakObjectPtr<UGameStateTeamComponent>(this));
	FAVVMGameThreadLock::FAVVMScopedLock ScopedLock = SynchronizationLock.Make(Callback);

	if (!ensureAlways(bWasSuccess))
	{
		return;
	}

	for (auto Iterator = Teams.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
	}

	TArray<UTeamObject*> OutTeam = MoveTemp(Teams);
	TArray<TWeakObjectPtr<const APlayerState>> OldUnassignedPlayers = MoveTemp(PendingPlayerStates);
	UTeamUtils::CreateOrAppendTeams(this, OldUnassignedPlayers, NewParties, TeamRule.Get(), OutTeam);

	Teams.Reset(OutTeam.Num());
	for (UTeamObject* NewTeam : OutTeam)
	{
		AddReplicatedSubObject(NewTeam);
		Teams.Add(NewTeam);
	}
}

void UGameStateTeamComponent::OnRep_OnTeamChanged(const TArray<UTeamObject*>& OldTeams)
{
	for (const UTeamObject* OldTeam : OldTeams)
	{
		if (!IsValid(OldTeam))
		{
			continue;
		}

		FAVVMNotificationContextArgs ContextArgs;
		ContextArgs.ChannelTag = TAG_TEAM_DESTROYED_NOTIFICATION;
		ContextArgs.Payload = OldTeam->GetTeamPayload();
		ContextArgs.Target = nullptr;
		UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
	}

	for (const UTeamObject* NewTeam : Teams)
	{
		if (!IsValid(NewTeam))
		{
			continue;
		}

		FAVVMNotificationContextArgs ContextArgs;
		ContextArgs.ChannelTag = TAG_TEAM_CREATED_NOTIFICATION;
		ContextArgs.Payload = NewTeam->GetTeamPayload();
		ContextArgs.Target = nullptr;
		UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
	}
}
