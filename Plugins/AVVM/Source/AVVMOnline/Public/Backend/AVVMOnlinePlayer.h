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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "AVVMNotificationSubsystem.h"

#include "AVVMOnlinePlayer.generated.h"

/**
 *	Class description:
 *
 *	FAVVMPlayerLoginContext encapsulate information about the user input when trying to login. This POD defines the data layout stored
 *	on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerLoginContext : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerLoginContext& Rhs) const;
	bool operator!=(const FAVVMPlayerLoginContext& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Username = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Password = FString();
};

/*
 *	Class description:
 *
*	FAVVMPlayerAccount is a POD representation of the player Account and is Unique. This POD defines the data layout stored
 *	on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerAccount
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerAccount& Rhs) const;
	bool operator!=(const FAVVMPlayerAccount& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMPlayerLoginContext.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 LoginId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Gamertag = FString();

	// @gdemers {FAVVMPlayerWallet.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 WalletId = INDEX_NONE;

	// @gdemers {FAVVMPlayerProfile.UniqueId} keep id reference for all owned profiles.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> ProfileIds;

	// @gdemers {FAVVMPlayerPreset.UniqueId} may refer to a complex system that captures details about the playable character builds.
	// Id referral allows preset sharing across profiles.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> PresetIds;
};

/*
 *	Class description:
 *
*	FAVVMPlayerWallet is a POD representation of a player account IRL money. (This includes Crypto, Gems, Token, etc...) This POD defines the data layout stored
 *	on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerWallet
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerWallet& Rhs) const;
	bool operator!=(const FAVVMPlayerWallet& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMCurrency} collection of currencies tied to player account.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> IrlMoneys;
};

/*
 *	Class description:
 *
 *	FAVVMCurrency is the POD representation of a currency. IRL or URL currency.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMCurrency
{
	GENERATED_BODY()

	bool operator==(const FAVVMCurrency& Rhs) const;
	bool operator!=(const FAVVMCurrency& Rhs) const;

	// @gdemers {FDataRegistryId}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString CurrencyId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 TotalAmount = INDEX_NONE;
};

/**
 *	Class description:
 *
*	FAVVMPlayerProfile is a POD representation of a playable character. This POD defines the data layout stored
 *	on the backend.
 *
 *	Note : A player profile is NOT an account. An account could hold various Player profiles. These are tied to a
 *	Character that's playable by the account holder. (It can also be seen as a save file/or session data.)
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerProfile
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerProfile& Rhs) const;
	bool operator!=(const FAVVMPlayerProfile& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your playable character.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ProfileId = FString();

	// @gdemers may refer to a complex system that captures progression details of items, skills, achievements, challenges, etc...
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Progression = FString();

	// @gdemers {FAVVMActorContent.UniqueId}. TArray<int32> is used for scenarios like "Resident Evil" where story branching prevents
	// Character.A items to be available during Character.B playtime. Storing unique instances here is required!
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> InventoryIds;

	// @gdemers {FAVVMPlayerChallenge.UniqueId} store shared challenges by id that are active for this profile.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> ChallengeIds;

	// @gdemers {FAVVMPlayerPreset.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 EquippedPresetId = INDEX_NONE;
};

/**
 *	Class description:
 *
*	FAVVMPlayerPreset is a POD representation of a player preset. This is a build that can equip all items referenced using their id. This POD defines the data layout stored
 *	on the backend.
 *	
 *	Note : It's possible that this POD type be only serialized to disk. Up to the user!
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerPreset
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerPreset& Rhs) const;
	bool operator!=(const FAVVMPlayerPreset& Rhs) const;

	// @gdemers uniquely identified to allow access between profiles owned by player account.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your profile preset.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PresetId = FString();

	// @gdemers {FAVVMItem.UniqueId}.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> EquippedItems;
};

/**
 *	Class description:
 *
*	FAVVMPlayerResource is a POD representation of resources that can be owned by a player profile, consumed, bought, sell, etc... This POD defines the data layout stored
 *	on the backend.
 *	
 *	example : Gear, Ammunition, Skills, Potions, etc...
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerResource : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerResource& Rhs) const;
	bool operator!=(const FAVVMPlayerResource& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistryId}, {Any information about a Resource}, i.e {Price}, {ResellPrice}, {Stats}, {MaxCapacity}, {CanStack}, etc...
	// Note : Live Services may have an actual POD that defines the properties from an excel sheet or something instead
	// of RegistryId.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ResourceId = FString();
};

/**
 *	Class description:
 *
*	FAVVMPlayerChallenge is a POD representation of a challenge. It contains details information WITHOUT progression. This POD defines the data layout stored
 *	on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerChallenge : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerChallenge& Rhs) const;
	bool operator!=(const FAVVMPlayerChallenge& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistryId}. {Goal}, TArray{Rewards}
	// Note : Live Services may have an actual POD that defines the properties from an excel sheet or something instead
	// of RegistryId.
	// Progression tracking should be handled separately.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ChallengeId = FString();
};

/**
 *	Class description:
 *
 *	EAVVMPlayerStatus track player state pre-gameplay. This is used before any seamless travel is invoked to sync players.
 */
UENUM(BlueprintType)
enum class EAVVMPlayerStatus : uint8
{
	Default,
	Ready,
	PendingAction
};

inline const TCHAR* EnumToString(EAVVMPlayerStatus State)
{
	switch (State)
	{
		case EAVVMPlayerStatus::Default:
			return TEXT("Default");
		case EAVVMPlayerStatus::Ready:
			return TEXT("Ready");
		case EAVVMPlayerStatus::PendingAction:
			return TEXT("PendingAction");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	FAVVMParty is a POD representation of a group of player connections. It exposed information about the session configuration
*	to all client connected. This POD defines the data layout stored on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMParty
{
	GENERATED_BODY()

	bool operator==(const FAVVMParty& Rhs) const;
	bool operator!=(const FAVVMParty& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may represent a party name.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PartyId = FString();

	// @gdemers NA, China, Russia, etc...
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 RegionId = INDEX_NONE;

	// @gdemers sub-identifier to the region.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 DistrictId = INDEX_NONE;

	// @gdemers {FAVVMHostConfiguration.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 HostConfigurationId = INDEX_NONE;

	// @gdemers {FAVVMPlayerConnection.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> PlayerConnectionIds;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerConnection is a POD representation of a player being connected with a party. It exposes information about the
*	user to other connected members. This POD defines the data layout stored on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerConnection
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerConnection& Rhs) const;
	bool operator!=(const FAVVMPlayerConnection& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers convert using FUniqueNetIdString::Create()
	UPROPERTY(Transient, BlueprintReadWrite)
	FString UniqueNetId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMPlayerStatus PlayerStatus = EAVVMPlayerStatus::Default;

	// @gdemers {FAVVMPlayerProfile.UniqueId} keep id reference for selected profile.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 ProfileId = INDEX_NONE;
};

/**
 *	Class description:
 *
*	FAVVMHostConfiguration is a POD representation of the Session configuration. This POD defines the data layout stored
 *	on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMHostConfiguration
{
	GENERATED_BODY()

	bool operator==(const FAVVMHostConfiguration& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers define the experience the players of a party will go through.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString GameMode = FString();

	// @gdemers may define complex properties tied to the gameplay experience to be run.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Options = FString();
};
