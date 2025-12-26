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

	// @gdemers we may not yet have a APlayerState bound to our Server PC but the FUniqueNetId is valid here.
	FUniqueNetIdPtr UniqueNetIdPtr = UniqueId.GetV1();
	if (UniqueNetIdPtr.IsValid())
	{
		AddPlayer(UniqueNetIdPtr->ToString());
	}
}

void AAVVMGameSession::UnregisterPlayer(const APlayerController* ExitingPlayer)
{
	Super::UnregisterPlayer(ExitingPlayer);

	const FString UniqueNetId = UAVVMOnlineUtils::GetUniqueNetId(IsValid(ExitingPlayer) ? ExitingPlayer->PlayerState : nullptr);
	if (!UniqueNetId.IsEmpty())
	{
		RemovePlayer(UniqueNetId);
	}
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

void AAVVMGameSession::AddPlayer(const FString& UniqueNetId)
{
	int32& OutPlayerConnectionId = PlayerConnectionIds.FindOrAdd(UniqueNetId);
	OutPlayerConnectionId = ResolveNewPlayerConnection(UniqueNetId);
}

void AAVVMGameSession::RemovePlayer(const FString& UniqueNetId)
{
	const bool bDoesContains = PlayerConnectionIds.Contains(UniqueNetId);
	if (bDoesContains)
	{
		const int32 PlayerConnectionId = PlayerConnectionIds[UniqueNetId];
		CleanupOldPlayerConnection(PlayerConnectionId);
		PlayerConnectionIds.Remove(UniqueNetId);
	}
}

int32 AAVVMGameSession::ResolveNewPlayerConnection(const FString& UniqueNetId)
{
	// @gdemers TODO make backend request that return a player connection id based on
	// the available FUniqueNetId.
	return INDEX_NONE;
}

void AAVVMGameSession::CleanupOldPlayerConnection(const int32 PlayerConnectionId)
{
	const bool bHasProfile = ProfileIds.Contains(PlayerConnectionId);
	if (!bHasProfile)
	{
		return;
	}

	const int32 ProfileId = ProfileIds[PlayerConnectionId];
	if (!ensureAlwaysMsgf(ProfileId != INDEX_NONE,
	                      TEXT("Cannot remove the requested Id from the collection. Missing entry.")))
	{
		return;
	}

	const bool bHasPreset = PresetIds.Contains(ProfileId);
	if (!bHasPreset)
	{
		return;
	}

	const int32 PresetId = PresetIds[ProfileId];
	if (!ensureAlwaysMsgf(PresetId != INDEX_NONE,
	                      TEXT("Cannot remove the requested Id from the collection. Missing entry.")))
	{
		return;
	}

	ProfileIds.Remove(PlayerConnectionId);
	PresetIds.Remove(ProfileId);
	ResolvedProfiles.Remove(ProfileId);
	ResolvedPresets.Remove(PresetId);
}
