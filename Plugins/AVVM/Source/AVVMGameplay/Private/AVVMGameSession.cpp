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
#include "AVVMGameSession.h"

#include "AVVMOnlineUtils.h"
#include "Backend/AVVMOnlinePlayer.h"

int32 AAVVMGameSession::Static_GetPlayerConnectionId(const UWorld* World,
                                                     const APlayerState* PlayerState)
{
	const AAVVMGameSession* GameSession = Get(World);
	return IsValid(GameSession) ? GameSession->GetPlayerConnectionId(PlayerState) : INDEX_NONE;
}

int32 AAVVMGameSession::Static_GetPlayerProfileId(const UWorld* World,
                                                  const APlayerState* PlayerState)
{
	const AAVVMGameSession* GameSession = Get(World);
	return IsValid(GameSession) ? GameSession->GetPlayerProfileId(PlayerState) : INDEX_NONE;
}

int32 AAVVMGameSession::Static_GetPlayerPresetId(const UWorld* World,
                                                 const APlayerState* PlayerState)
{
	const AAVVMGameSession* GameSession = Get(World);
	return IsValid(GameSession) ? GameSession->GetPlayerPresetId(PlayerState) : INDEX_NONE;
}

TArray<int32> AAVVMGameSession::Static_GetPlayerPresetItems(const UWorld* World,
                                                            const int32 ProfileId)
{
	const AAVVMGameSession* GameSession = Get(World);
	return IsValid(GameSession) ? GameSession->GetPlayerPresetItems(ProfileId) : TArray<int32>{};
}

void AAVVMGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
	Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);

	// TODO @gdemers Resolve newly connected player {FAVVMPlayerConnection}, {FAVVMPlayerProfile}
	// i.e the selected profile equipped by the new party member.
}

void AAVVMGameSession::UnregisterPlayer(const APlayerController* ExitingPlayer)
{
	Super::UnregisterPlayer(ExitingPlayer);
}

AAVVMGameSession* AAVVMGameSession::Get(const UWorld* World)
{
	return Cast<AAVVMGameSession>(UAVVMOnlineUtils::GetGameSession(World));
}

int32 AAVVMGameSession::GetPlayerConnectionId(const APlayerState* PlayerState) const
{
	const FString UniqueNetId = UAVVMOnlineUtils::GetUniqueNetId(PlayerState);

	const bool bDoesContains = PlayerConnectionIds.Contains(UniqueNetId);
	if (ensureAlwaysMsgf(bDoesContains,
	                     TEXT("Entry missing in collection.")))
	{
		return PlayerConnectionIds[UniqueNetId];
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 AAVVMGameSession::GetPlayerProfileId(const APlayerState* PlayerState) const
{
	const int32 PlayerConnectionId = GetPlayerConnectionId(PlayerState);
	if (PlayerConnectionId != INDEX_NONE)
	{
		const bool bDoesContains = ProfileIds.Contains(PlayerConnectionId);
		if (ensureAlwaysMsgf(bDoesContains,
		                     TEXT("Entry missing in collection.")))
		{
			return ProfileIds[PlayerConnectionId];
		}
		else
		{
			return INDEX_NONE;
		}
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 AAVVMGameSession::GetPlayerPresetId(const APlayerState* PlayerState) const
{
	const int32 PlayerProfileId = GetPlayerProfileId(PlayerState);
	if (PlayerProfileId != INDEX_NONE)
	{
		const bool bDoesContains = PresetIds.Contains(PlayerProfileId);
		if (ensureAlwaysMsgf(bDoesContains,
		                     TEXT("Entry missing in collection.")))
		{
			return PresetIds[PlayerProfileId];
		}
		else
		{
			return INDEX_NONE;
		}
	}
	else
	{
		return INDEX_NONE;
	}
}

TArray<int32> AAVVMGameSession::GetPlayerPresetItems(const int32 ProfileId) const
{
	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return TArray<int32>{};
	}

	const bool bDoesContains = PresetIds.Contains(ProfileId);
	if (bDoesContains)
	{
		const int32 PresetId = PresetIds[ProfileId];

		const bool bHasResolvedPreset = ResolvedPresets.Contains(PresetId);
		if (!ensureAlwaysMsgf(bHasResolvedPreset,
		                      TEXT("Cannot resolve the Backend representation referenced by the provided Id.")))
		{
			return TArray<int32>{};
		}

		const FString PresetPayload = ResolvedPresets[PresetId];

		FAVVMPlayerPreset OutPlayerPreset;
		JsonParser->FromString(PresetPayload, OutPlayerPreset);

		return OutPlayerPreset.EquippedItems;
	}
	else
	{
		return TArray<int32>{};
	}
}
