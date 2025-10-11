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
#include "AVVMOnlineStringParser.h"
#include "PlayerStateTeamComponent.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

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

void UTeamObject::SetTeam(const FGameplayTag& NewTeamTag)
{
	TeamTag = NewTeamTag;
}

void UTeamObject::RegisterPlayer(const APlayerState* NewPlayer, const FString& NewPlayerUniqueNetId)
{
	PlayerUniqueNetIds.AddUnique(NewPlayerUniqueNetId);
	
	auto* TeamComponent = UPlayerStateTeamComponent::GetActorComponent(NewPlayer);
	if (IsValid(TeamComponent))
	{
		TeamComponent->SetTeam(this);
	}
}

UTeamObject* UTeamObject::Factory(UObject* Outer,
                                  const FAVVMPartyProxy& PartyProxy,
                                  const TArray<TWeakObjectPtr<APlayerState>>& Players)
{
	UTeamObject* NewTeam = NewObject<UTeamObject>(Outer);
	if (!IsValid(NewTeam))
	{
		return nullptr;
	}

	UAVVMOnlineStringParser* OnlineStringParser = FAVVMOnlineModule::GetJsonParser();
	if (!IsValid(OnlineStringParser))
	{
		return nullptr;
	}

	for (const FString& PlayerConnection : PartyProxy.PlayerConnections)
	{
		FAVVMPlayerConnectionProxy OutPlayerProxy;
		OnlineStringParser->FromString(PlayerConnection, OutPlayerProxy);

		// TODO @gdemers Warning. I am no longer certain that the UniqueNetId would be persistent from lobby to gameplay after the client has travels based on
		// server configuration. TBD!
		const auto* SearchResult = Players.FindByPredicate([SearchNetId = OutPlayerProxy.UniqueNetId](const TWeakObjectPtr<APlayerState>& Player)
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
			NewTeam->RegisterPlayer(SearchResult->Get(), OutPlayerProxy.UniqueNetId);
		}
	}

	return NewTeam;
}

void UTeamObject::OnRep_OnTeamCompositionChanged(const TArray<FString>& OldPlayerUniqueNetIds)
{
	// TODO @gdemers notify external system of state change.
}
