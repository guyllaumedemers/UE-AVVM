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
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

int32 AAVVMGameSession::Static_GetPlayerConnectionId(const UObject* WorldContextObject,
                                                     const APlayerState* PlayerState)
{
	const AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetPlayerConnectionId(PlayerState) : INDEX_NONE;
}

int32 AAVVMGameSession::Static_GetPlayerProfileId(const UObject* WorldContextObject,
                                                  const APlayerState* PlayerState)
{
	const AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetPlayerProfileId(PlayerState) : INDEX_NONE;
}

int32 AAVVMGameSession::Static_GetPlayerPresetId(const UObject* WorldContextObject,
                                                 const APlayerState* PlayerState)
{
	const AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetPlayerPresetId(PlayerState) : INDEX_NONE;
}

TArray<int32> AAVVMGameSession::Static_GetPlayerPresetItems(const UObject* WorldContextObject,
                                                            const int32 ProfileId)
{
	const AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetPlayerPresetItems(ProfileId) : TArray<int32>{};
}

TArray<int32> AAVVMGameSession::Static_GetPlayerInventoryItems(const UObject* WorldContextObject,
                                                               const int32 ProfileId)
{
	const AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetPlayerInventoryItems(ProfileId) : TArray<int32>{};
}

TArray<int32> AAVVMGameSession::Static_GetActorInventoryItems(const UObject* WorldContextObject,
                                                              const int32 ProfileId)
{
	const AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetActorInventoryItems(ProfileId) : TArray<int32>{};
}

FString AAVVMGameSession::Static_ModifyPlayerProfileInventory(const UObject* WorldContextObject,
                                                              const int32 ProfileId,
                                                              const TArray<int32>& NewItems)
{
	AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->ModifyPlayerProfileInventory(ProfileId, NewItems) : FString();
}

FGameplayTag AAVVMGameSession::Static_GetPlayerPresetSlot(const UObject* WorldContextObject,
                                                          const int32 ProfileId,
                                                          const int32 PrivateItemId)
{
	AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetPlayerPresetSlot(ProfileId, PrivateItemId) : FGameplayTag::EmptyTag;
}

FGameplayTag AAVVMGameSession::Static_GetActorPresetSlot(const UObject* WorldContextObject,
                                                         const int32 ProfileId,
                                                         const int32 PrivateItemId)
{
	AAVVMGameSession* GameSession = Get(WorldContextObject);
	return IsValid(GameSession) ? GameSession->GetActorPresetSlot(ProfileId, PrivateItemId) : FGameplayTag::EmptyTag;
}

void AAVVMGameSession::RegisterPlayer(APlayerController* NewPlayer,
                                      const FUniqueNetIdRepl& UniqueId,
                                      bool bWasFromInvite)
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

AAVVMGameSession* AAVVMGameSession::Get(const UObject* WorldContextObject)
{
	const AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(WorldContextObject);
	return IsValid(GameModeBase) ? Cast<AAVVMGameSession>(GameModeBase->GameSession) : nullptr;
}

int32 AAVVMGameSession::GetPlayerConnectionId(const APlayerState* PlayerState) const
{
	const FString UniqueNetId = UAVVMOnlineUtils::GetUniqueNetId(PlayerState);

	const bool bDoesContains = SessionPayload.PlayerConnectionIds.Contains(UniqueNetId);
	if (ensureAlwaysMsgf(bDoesContains,
	                     TEXT("Entry missing in collection.")))
	{
		return SessionPayload.PlayerConnectionIds[UniqueNetId];
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
		const bool bDoesContains = SessionPayload.ProfileIds.Contains(PlayerConnectionId);
		if (ensureAlwaysMsgf(bDoesContains,
		                     TEXT("Entry missing in collection.")))
		{
			return SessionPayload.ProfileIds[PlayerConnectionId];
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
		const bool bDoesContains = SessionPayload.PresetIds.Contains(PlayerProfileId);
		if (ensureAlwaysMsgf(bDoesContains,
		                     TEXT("Entry missing in collection.")))
		{
			return SessionPayload.PresetIds[PlayerProfileId];
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

	const bool bDoesContains = SessionPayload.PresetIds.Contains(ProfileId);
	if (bDoesContains)
	{
		const int32 PresetId = SessionPayload.PresetIds[ProfileId];

		const bool bHasResolvedPreset = SessionPayload.ResolvedPresets.Contains(PresetId);
		if (!ensureAlwaysMsgf(bHasResolvedPreset,
		                      TEXT("Cannot resolve the Backend representation referenced by the provided Id.")))
		{
			return TArray<int32>{};
		}

		const FString PresetPayload = SessionPayload.ResolvedPresets[PresetId];

		FAVVMPlayerPreset OutPlayerPreset;
		JsonParser->FromString(PresetPayload, OutPlayerPreset);

		TArray<int32> OutEquippedItems;
		OutPlayerPreset.EquippedItems.GenerateValueArray(OutEquippedItems);

		return OutEquippedItems;
	}
	else
	{
		return TArray<int32>{};
	}
}

TArray<int32> AAVVMGameSession::GetPlayerInventoryItems(const int32 ProfileId) const
{
	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return TArray<int32>{};
	}

	const bool bHasResolvedProfile = SessionPayload.ResolvedProfiles.Contains(ProfileId);
	if (!ensureAlwaysMsgf(bHasResolvedProfile,
	                      TEXT("Cannot resolve the Backend representation referenced by the provided Id.")))
	{
		return TArray<int32>{};
	}

	const FString ProfilePayload = SessionPayload.ResolvedProfiles[ProfileId];

	FAVVMPlayerProfile OutPlayerProfile;
	JsonParser->FromString(ProfilePayload, OutPlayerProfile);

	return OutPlayerProfile.InventoryIds;
}

TArray<int32> AAVVMGameSession::GetActorInventoryItems(const int32 ProfileId) const
{
	// TODO @gdemers access backend representation of our actor, and their inventory
	// this may apply to NPC types, Shops, Boxes, etc... 
	return TArray<int32>{};
}

FString AAVVMGameSession::ModifyPlayerProfileInventory(const int32 ProfileId,
                                                       const TArray<int32>& NewItems)
{
	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return FString();
	}

	const bool bHasResolvedProfile = SessionPayload.ResolvedProfiles.Contains(ProfileId);
	if (!ensureAlwaysMsgf(bHasResolvedProfile,
	                      TEXT("Cannot resolve the Backend representation referenced by the provided Id.")))
	{
		return FString();
	}

	FAVVMPlayerProfile OutOldProfile;

	FString& OldProfile = SessionPayload.ResolvedProfiles[ProfileId];
	JsonParser->FromString(OldProfile, OutOldProfile);

	// @gdemers dirty profile with new data.
	OutOldProfile.InventoryIds = NewItems;
	JsonParser->ToString(OutOldProfile, OldProfile);

	return OldProfile;
}

FGameplayTag AAVVMGameSession::GetPlayerPresetSlot(const int32 ProfileId,
                                                   const int32 PrivateItemId)
{
	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return FGameplayTag::EmptyTag;
	}

	const bool bHasResolvedProfile = SessionPayload.ResolvedProfiles.Contains(ProfileId);
	if (!ensureAlwaysMsgf(bHasResolvedProfile,
	                      TEXT("Cannot resolve the Backend representation referenced by the provided Id.")))
	{
		return FGameplayTag::EmptyTag;
	}

	const FString ProfilePayload = SessionPayload.ResolvedProfiles[ProfileId];

	FAVVMPlayerProfile OutPlayerProfile;
	JsonParser->FromString(ProfilePayload, OutPlayerProfile);

	const bool bHasResolvedPreset = SessionPayload.ResolvedPresets.Contains(OutPlayerProfile.EquippedPresetId);
	if (!ensureAlwaysMsgf(bHasResolvedPreset,
	                      TEXT("Cannot resolve the Backend representation referenced by the provided Id.")))
	{
		return FGameplayTag::EmptyTag;
	}

	const FString PresetPayload = SessionPayload.ResolvedPresets[OutPlayerProfile.EquippedPresetId];

	FAVVMPlayerPreset OutPlayerPreset;
	JsonParser->FromString(PresetPayload, OutPlayerPreset);

	const FGameplayTag* SearchResult = OutPlayerPreset.EquippedItems.FindKey(PrivateItemId);
	return (SearchResult != nullptr) ? *SearchResult : FGameplayTag::EmptyTag;
}

FGameplayTag AAVVMGameSession::GetActorPresetSlot(const int32 ProfileId,
                                                  const int32 PrivateItemId)
{
	// TODO @gdemers access backend representation of our actor, and their inventory
	// this may apply to NPC types, Shops, Boxes, etc...
	return FGameplayTag::EmptyTag;
}

void AAVVMGameSession::AddPlayer(const FString& UniqueNetId)
{
	int32& OutPlayerConnectionId = SessionPayload.PlayerConnectionIds.FindOrAdd(UniqueNetId);
	OutPlayerConnectionId = ResolveNewPlayerConnection(UniqueNetId);
}

void AAVVMGameSession::RemovePlayer(const FString& UniqueNetId)
{
	const bool bDoesContains = SessionPayload.PlayerConnectionIds.Contains(UniqueNetId);
	if (bDoesContains)
	{
		const int32 PlayerConnectionId = SessionPayload.PlayerConnectionIds[UniqueNetId];
		CleanupOldPlayerConnection(PlayerConnectionId);
		SessionPayload.PlayerConnectionIds.Remove(UniqueNetId);
	}
}

int32 AAVVMGameSession::ResolveNewPlayerConnection(const FString& UniqueNetId)
{
	// @gdemers TODO make backend request that return a player connection id based on
	// the available FUniqueNetId.
	// @gdemers truncated value, will def cause problem eventually! temp solution for testing in PIE.
	const int32 TypeHash = GetTypeHash(UniqueNetId);
	return TypeHash;
}

void AAVVMGameSession::CleanupOldPlayerConnection(const int32 PlayerConnectionId)
{
	const bool bHasProfile = SessionPayload.ProfileIds.Contains(PlayerConnectionId);
	if (!bHasProfile)
	{
		return;
	}

	const int32 ProfileId = SessionPayload.ProfileIds[PlayerConnectionId];
	if (!ensureAlwaysMsgf(ProfileId != INDEX_NONE,
	                      TEXT("Cannot remove the requested Id from the collection. Missing entry.")))
	{
		return;
	}

	const bool bHasPreset = SessionPayload.PresetIds.Contains(ProfileId);
	if (!bHasPreset)
	{
		return;
	}

	const int32 PresetId = SessionPayload.PresetIds[ProfileId];
	if (!ensureAlwaysMsgf(PresetId != INDEX_NONE,
	                      TEXT("Cannot remove the requested Id from the collection. Missing entry.")))
	{
		return;
	}

	SessionPayload.ProfileIds.Remove(PlayerConnectionId);
	SessionPayload.PresetIds.Remove(ProfileId);
	SessionPayload.ResolvedProfiles.Remove(ProfileId);
	SessionPayload.ResolvedPresets.Remove(PresetId);
}
