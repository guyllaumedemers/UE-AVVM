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
#include "AVVMGameState.h"
#include "AVVMWorldSetting.h"
#include "NativeGameplayTags.h"
#include "PlayerStateTeamComponent.h"
#include "TeamObject.h"
#include "TeamRule.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_WORLD_RULE_TEAM, "WorldRule.Team");

UGameStateTeamComponent::UGameStateTeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bReplicateUsingRegisteredSubObjectList = false;
}

void UGameStateTeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameStateTeamComponent, Teams);
}

void UGameStateTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	UPlayerStateTeamComponent::OnTeamComponentInitialized.AddUObject(this, &UGameStateTeamComponent::OnPlayerStateTeamComponentInitialized);

	auto* Outer = GetTypedOuter<AAVVMGameState>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UGameStateTeamComponent to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	if (Outer->HasAuthority())
	{
		GetTeamRuleOnAuthority();
		Outer->OnPlayerStateRemoved.AddUObject(this, &UGameStateTeamComponent::OnPlayerStateRemoved);
		Outer->OnPlayerStateAdded.AddUObject(this, &UGameStateTeamComponent::OnPlayerStateAdded);
	}

	OwningOuter = Outer;
}

void UGameStateTeamComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UPlayerStateTeamComponent::OnTeamComponentInitialized.RemoveAll(this);

	auto* Outer = Cast<AAVVMGameState>(OwningOuter.Get());
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UGameStateTeamComponent from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	if (Outer->HasAuthority())
	{
		Outer->OnPlayerStateRemoved.RemoveAll(this);
		Outer->OnPlayerStateAdded.RemoveAll(this);
	}
}

void UGameStateTeamComponent::OnPlayerStateAdded(APlayerState* NewPlayerState)
{
	PendingPlayers.Add(NewPlayerState);
}

void UGameStateTeamComponent::OnPlayerStateRemoved(APlayerState* NewPlayerState)
{
	PendingPlayers.Remove(NewPlayerState);
}

void UGameStateTeamComponent::OnPlayerStateTeamComponentInitialized(APlayerState* NewPlayerState)
{
	PendingPlayers.Add(NewPlayerState);
}

void UGameStateTeamComponent::GetTeamRuleOnAuthority()
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const auto* WorldSetting = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (IsValid(WorldSetting))
	{
		TeamRule = WorldSetting->CastRule<UTeamRule>(TAG_WORLD_RULE_TEAM);
	}

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
		BP_RequestGameSesionParties(GameSession->SessionName, Callback);
	}
}

void UGameStateTeamComponent::OnTeamReceived(const bool bWasSuccess,
                                             const TArray<FAVVMPartyProxy>& NewParties)
{
	if (!ensureAlwaysMsgf(bWasSuccess,
	                      TEXT("Backend couldnt retrieve involved parties.")))
	{
		return;
	}

	const UTeamRule* Rule = TeamRule.Get();
	if (!IsValid(Rule))
	{
		return;
	}

	TArray<FGameplayTag> DuplicatedTags = Rule->GetTags();
	Teams.Reset(NewParties.Num());

	TArray<TWeakObjectPtr<APlayerState>> Players = MoveTemp(PendingPlayers);
	for (const FAVVMPartyProxy& Party : NewParties)
	{
		auto* TeamObject = UTeamObject::Factory(OwningOuter.Get(), Party, Players);
		if (!IsValid(TeamObject))
		{
			continue;
		}

		if (ensureAlwaysMsgf(!DuplicatedTags.IsEmpty(),
		                     TEXT("TeamTags were not configured. Attempting assignment on Empty set.")))
		{
			const FGameplayTag TeamTag = DuplicatedTags.Pop();
			TeamObject->SetTeam(TeamTag);
		}

		Teams.Add(TeamObject);
	}
}

void UGameStateTeamComponent::OnRep_OnTeamChanged(const TArray<UTeamObject*>& OldTeams)
{
	OnReplicatedTeamChanged.Broadcast(Teams, OldTeams);
}
