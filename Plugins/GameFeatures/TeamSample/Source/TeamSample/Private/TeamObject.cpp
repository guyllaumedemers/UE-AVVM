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
#include "TeamObject.h"

#include "AVVMOnline.h"
#include "AVVMOnlineInterfaceUtils.h"
#include "AVVMOnlineStringParser.h"
#include "NativeGameplayTags.h"
#include "PlayerStateTeamComponent.h"
#include "TeamRule.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_TEAM_COMPOSITION_CHANGED_NOTIFICATION, "TeamSample.Notification.TeamCompositionChanged");

FTeamPayload::FTeamPayload(const TArray<FString>& NewPlayerUniqueNetIds,
                           const FGameplayTag& NewTeamTag)
	: TeamTag(NewTeamTag),
	  PlayerUniqueNetIds(NewPlayerUniqueNetIds)
{
}

void UTeamObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeamObject, PlayerUniqueNetIds);
}

bool UTeamObject::IsSupportedForNetworking() const
{
	return true;
}

#if UE_WITH_IRIS
void UTeamObject::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicaitonFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

TInstancedStruct<FTeamPayload> UTeamObject::GetTeamPayload() const
{
	return FTeamPayload::Make<FTeamPayload>(PlayerUniqueNetIds, TeamTag);
}

void UTeamObject::RegisterPlayerState(const APlayerState* NewPlayerState, const FString& NewPlayerUniqueNetId)
{
	PlayerUniqueNetIds.AddUnique(NewPlayerUniqueNetId);
	
	auto* TeamComponent = UPlayerStateTeamComponent::GetActorComponent(NewPlayerState);
	if (IsValid(TeamComponent))
	{
		TeamComponent->SetTeam(this);
	}
}

void UTeamObject::UnRegisterPlayerState(const APlayerState* OldPlayerState)
{
	const FString PlayerUniqueNetId = UAVVMOnlineInterfaceUtils::GetUniqueNetId(OldPlayerState);
	PlayerUniqueNetIds.Remove(PlayerUniqueNetId);
}

void UTeamObject::OnRep_OnTeamCompositionChanged(const TArray<FString>& OldPlayerUniqueNetIds)
{
	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_TEAM_COMPOSITION_CHANGED_NOTIFICATION;
	ContextArgs.Payload = GetTeamPayload();
	ContextArgs.Target = nullptr;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void UTeamUtils::CreateOrAppendTeams(UObject* Outer,
                                     const TArray<TWeakObjectPtr<const APlayerState>>& UnassignedPlayerStates,
                                     const TArray<FAVVMPartyProxy>& NewParties,
                                     const UTeamRule* Rule,
                                     TArray<UTeamObject*>& OutTeams)
{
	if (!IsValid(Rule))
	{
		return;
	}

	TArray<FGameplayTag> DuplicatedTags = Rule->GetTags();
	bool bHasInitialized = true;
	
	if (OutTeams.IsEmpty())
	{
		OutTeams.Reset(NewParties.Num());
		bHasInitialized = false;
	}

	for (const FAVVMPartyProxy& Party : NewParties)
	{
		if (!bHasInitialized)
		{
			UTeamObject* NewTeam = UTeamUtils::CreateTeam(Outer, UnassignedPlayerStates, Party);
			if (!IsValid(NewTeam))
			{
				continue;
			}

			if (ensureAlwaysMsgf(!DuplicatedTags.IsEmpty(),
			                     TEXT("TeamTags were not configured. Attempting assignment on Empty set.")))
			{
				NewTeam->TeamTag = DuplicatedTags.Pop();
				NewTeam->PartyUniqueId = Party.UniqueId;
				OutTeams.Add(NewTeam);
			}
		}
		else
		{
			UTeamObject* OldTeam = UTeamUtils::FindTeam(OutTeams, Party.UniqueId);
			UTeamUtils::AppendTeam(UnassignedPlayerStates, Party.PlayerConnections, OldTeam);
		}
	}
}

UTeamObject* UTeamUtils::CreateTeam(UObject* Outer,
                                    const TArray<TWeakObjectPtr<const APlayerState>>& UnassignedPlayerStates,
                                    const FAVVMPartyProxy& NewParty)
{
	UTeamObject* NewTeam = NewObject<UTeamObject>(Outer);
	AppendTeam(UnassignedPlayerStates, NewParty.PlayerConnections, NewTeam);
	return NewTeam;
}

void UTeamUtils::AppendTeam(const TArray<TWeakObjectPtr<const APlayerState>>& UnassignedPlayerStates,
                            const TArray<FString>& NewPlayerConnections,
                            UTeamObject* NewTeam)
{
	if (!IsValid(NewTeam))
	{
		return;
	}

	UAVVMOnlineStringParser* OnlineStringParser = FAVVMOnlineModule::GetJsonParser();
	if (!IsValid(OnlineStringParser))
	{
		return;
	}

	for (const FString& PlayerConnection : NewPlayerConnections)
	{
		FAVVMPlayerConnectionProxy OutPlayerProxy;
		OnlineStringParser->FromString(PlayerConnection, OutPlayerProxy);

		const TWeakObjectPtr<const APlayerState>* SearchResult = UnassignedPlayerStates.FindByPredicate([SearchNetId = OutPlayerProxy.UniqueNetId](const TWeakObjectPtr<const APlayerState>& Player)
		{
			if (!Player.IsValid())
			{
				return false;
			}

			FUniqueNetIdPtr NetId = Player->GetUniqueId().GetUniqueNetId();
			if (NetId.IsValid())
			{
				return NetId->ToString().Equals(SearchNetId);
			}

			return false;
		});

		if (SearchResult != nullptr)
		{
			NewTeam->RegisterPlayerState(SearchResult->Get(), OutPlayerProxy.UniqueNetId);
		}
	}
}

UTeamObject* UTeamUtils::FindTeam(const TArray<UTeamObject*>& NewTeams,
                                  const int32 NewPartyUniqueId)
{
	UTeamObject* const* SearchResult = NewTeams.FindByPredicate([SearchPartyId = NewPartyUniqueId](const UTeamObject* NewTeam)
	{
		return IsValid(NewTeam) && (NewTeam->PartyUniqueId == SearchPartyId);
	});

	return (SearchResult != nullptr) ? *SearchResult : nullptr;
}

UTeamObject* UTeamUtils::FindTeam(const TArray<UTeamObject*>& NewTeams,
                                  const APlayerState* NewPlayerState)
{
	UTeamObject* const* SearchResult = NewTeams.FindByPredicate([SearchNetId = UAVVMOnlineInterfaceUtils::GetUniqueNetId(NewPlayerState)](const UTeamObject* NewTeam)
	{
		return IsValid(NewTeam) && (NewTeam->PlayerUniqueNetIds.Contains(SearchNetId));
	});

	return (SearchResult != nullptr) ? *SearchResult : nullptr;
}
