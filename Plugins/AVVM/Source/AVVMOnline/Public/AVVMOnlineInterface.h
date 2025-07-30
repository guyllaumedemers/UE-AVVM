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
#include "AVVMOnline.h"

#include "AVVMOnlineInterface.generated.h"

/**
 *	Class description:
 *
 *	FAVVMStringPayload encapsulate a string representation for a collection of user defined types. example : TArray<FAVVMPlayerConnections>. When unfold, the
 *	FString returned from backend should resolve like so.
 *
 *	example : TArray<FAVVMPlayerConnections> PlayerConnections = USomeUtilsApi::FromJson(InputStringPayload);
 *
 *	IMPORTANT : For single types, the Online Subsystem service should manage forwarding already resolved types. To maintain our system simple,
 *	a collection types should be resolved at the View Model level when receiving the "erase" type. i.e handled through TInstancedStruct<FAVVMNotificationPayload> which
 *	would in that case be resolved as such : TInstancedStruct<FAVVMNotificationPayload>()->GetPtr<FAVVMStringPayload>().
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMStringPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMStringPayload& Rhs) const;

	// @gdemers used to prevent creation of an explicit type wrapping a collection of user defined structs.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload = FString();
};

/*
 *	Class description:
 *
 *	FAVVMCurrency is the POD representation of a currency. IRL or URL currency.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMCurrency : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMCurrency& Rhs) const;

	// @gdemers {FDataRegistryId}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString CurrencyId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 TotalAmount = INDEX_NONE;
};

/*
 *	Class description:
 *
 *	FAVVMPlayerWallet is a POD representation of a player account IRL money. (This includes Crypto, Gems, Token, etc...)
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerWallet : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerWallet& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMCurrency} collection of currencies tied to player account.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> IrlMoneys;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerResource is a POD representation of resources that can be owned by a player profile, consumed, bought, sell, etc...
 *	example : Gear, Ammunition, Skills, Potions, etc...
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerResource : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerResource& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistry}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ResourceId = FString();

	// @gdmers may define complex properties such as {Price}, {ResellPrice}, {Stats}, etc...
	// Note : These SHOULD NOT store any profile specifics modifier applied. Only the base stats defined by GD. Custom mods should be stored separately.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Options = FString();
};

/**
 *	Class description:
 *
 *	FAVVMPlayerChallenge is a POD representation of a challenge. It contains details information WITHOUT progression.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerChallenge : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerChallenge& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistry}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ChallengeId = FString();

	// @gdemers progress should be handled specific to the player profile in the progression system.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 Goal = INDEX_NONE;

	// @gdmers {FAVVMPlayerResource.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> ResourceIds;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerPreset is a POD representation of a player preset. This is a build that can equip all items referenced using their id.
 *	Note : It's possible that this POD type be only serialized to disk. Up to the user!
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerPreset : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerPreset& Rhs) const;

	// @gdemers uniquely identified to allow access between profiles owned by player account.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistry}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PresetId = FString();

	// @gdemers {FAVVMPlayerResource.UniqueId} contains set of unique identifier to player owned items. example : Skills, Gear, etc...
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> EquippedItems;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerProfile is a POD representation of a playable character.
 *
 *	Note : A player profile is NOT an account. An account could hold various Player profiles. These are tied to a
 *	Character that's playable by the account holder.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerProfile : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerProfile& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your playable character.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ProfileId = FString();

	// @gdemers may refer to a complex system that captures progression details of items, skills, achievements, challenges, etc...
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Progression = FString();

	// @gdemers {InventorySample::FActorContent.UniqueId}. TArray<int32> is used for scenarios like "Resident Evil" where story branching prevents
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

/*
 *	Class description:
 *
 *	FAVVMPlayerAccount is a POD representation of the player Account and is Unique.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerAccount : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerAccount& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

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

/**
 *	Class description:
 *
 *	FAVVMHostConfiguration is a POD representation of the Session configuration.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMHostConfiguration : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMHostConfiguration& Rhs) const;

	// @gdemers define the experience the players of a party will go through.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString GameMode = FString();

	// @gdemers may define complex properties tied to the gameplay experience to be run.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Options = FString();
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
 *	FAVVMPlayerConnection is a POD representation of a player being connected with a party. It exposes information about the
 *	user to other connected members.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerConnection : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerConnection& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadOnly)
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
 *	FAVVMParty is a POD representation of a group of player connections. It exposed information about the session configuration
 *	to all client connected.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMParty : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMParty& Rhs) const;

	// @gdemers unique id to identify shared POD type. prevent entry duplication on backend.
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may represent a party name.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PartyId = FString();

	// @gdemers NA, China, Russia, etc...
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 RegionId = INDEX_NONE;

	// @gdemers sub-identifier to the region.
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 DistrictId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString HostConfiguration = FString();

	// @gdemers {FAVVMPlayerProfile.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> PlayerConnectionIds;
};

/**
 *	Class description:
 *
 *	EAVVMPlayerRequestType define a set of possible actions a request can execute.
 */
UENUM(BlueprintType)
enum class EAVVMPlayerRequestType : uint8
{
	None,
	AddFriend,
	RemoveFriend,
	InvitePlayer, // @gdemers more generic, not restricted to friends. Allow sending a request when two players are in the same world. (and not only via profile access)
	BlockPlayer,
	KickPlayer,
	MutePlayer,
	CensorPlayer,
	Trade
};

inline const TCHAR* EnumToString(EAVVMPlayerRequestType State)
{
	switch (State)
	{
		case EAVVMPlayerRequestType::AddFriend:
			return TEXT("AddFriend");
		case EAVVMPlayerRequestType::RemoveFriend:
			return TEXT("RemoveFriend");
		case EAVVMPlayerRequestType::InvitePlayer:
			return TEXT("InvitePlayer");
		case EAVVMPlayerRequestType::BlockPlayer:
			return TEXT("BlockPlayer");
		case EAVVMPlayerRequestType::KickPlayer:
			return TEXT("KickPlayer");
		case EAVVMPlayerRequestType::MutePlayer:
			return TEXT("MutePlayer");
		case EAVVMPlayerRequestType::CensorPlayer:
			return TEXT("CensorPlayer");
		case EAVVMPlayerRequestType::Trade:
			return TEXT("Trade");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	FAVVMPlayerRequest a request which target a player, remote or local, and execute an action.
 *
 *	example : kick player out of party, invite friend, etc...
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerRequest : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMPlayerRequest() = default;
	explicit FAVVMPlayerRequest(const FString& NewSrcPlayerUniqueNetId,
	                            const FString& NewDestPlayerUniqueNetId,
	                            const EAVVMPlayerRequestType NewRequestType,
	                            const FString& NewPayload);

	bool operator==(const FAVVMPlayerRequest& Rhs) const;

	// @gdemers caller.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString SrcPlayerUniqueNetId = FString();

	// @gdemers receiver.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString DestPlayerUniqueNetId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMPlayerRequestType RequestType = EAVVMPlayerRequestType::None;

	// @gdemers payload can represent any data sent over the network, maybe during a trade, etc...
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload = FString();
};

/**
 *	Class description:
 *
 *	FAVVMLoginContext encapsulate information about the user input when
 *	trying to login.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMLoginContext : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMLoginContext& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Username = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Password = FString();
};

/**
*	Class description:
 *
 *	UAVVMOnlineIdentityInterface is the subsystem interfacing with the microservice for tracking the player identity,
 *	login/logout action and profile updates.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineIdentityInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineIdentityInterface
{
	GENERATED_BODY()

public:
	virtual bool IsHosting(const FUniqueNetIdPtr PlayerUniqueNetIdPtr) const
	{
		return true;
	}

	virtual void RequestLogin(const FAVVMLoginContext& LoginContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	};

	virtual void CommitModifiedPlayerProfile(const FAVVMPlayerProfile& PlayerContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullPlayerProfile(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void CommitModifiedHostConfiguration(const FAVVMHostConfiguration& ConfigurationContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullHostConfiguration(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	// @gdemers networked request between two players.
	// Note : trading and sending gift are really the same action, the difference is simply in the payload exchange, one end
	// of the interaction being empty. re-use the same api if required!
	virtual void Trade(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};

/**
*	Class description:
 *
 *	UAVVMOnlinePartyInterface is the subsystem interfacing with the microservice for tracking available groups/party/sessions, etc...
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlinePartyInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlinePartyInterface
{
	GENERATED_BODY()

public:
	virtual void JoinParty(const FAVVMParty& PartyContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ExitParty(const FAVVMParty& PartyContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void KickFromParty(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullParties(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};

/**
*	Class description:
 *
 *	UAVVMOnlineFriendInterface is the subsystem interfacing with the microservice for a user Friends.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineFriendInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineFriendInterface
{
	GENERATED_BODY()

public:
	virtual void AddFriend(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void RemoveFriend(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void InvitePlayer(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void BlockPlayer(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};

/**
*	Class description:
 *
 *	UAVVMOnlineMessagingInterface is the subsystem interfacing with the microservice for messaging channels between players.
 *	(mic, chat, email, etc...)
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineMessagingInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineMessagingInterface
{
	GENERATED_BODY()

public:
	virtual void MutePlayer(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void CensorPlayer(const FAVVMPlayerRequest& PlayerRequestContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};

/**
*	Class description:
 *
 *	UAVVMOnlineChallengesInterface is the subsystem interfacing with the microservice for (Player) Challenges progression.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineChallengesInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineChallengesInterface
{
	GENERATED_BODY()

public:
	virtual void ClaimChallenge(const FAVVMPlayerChallenge& ChallengeContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullChallenges(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};

/**
 *	Class description:
 *
 *	UAVVMOnlineBattlePassInterface is the subsystem interfacing with the microservice for Battle Pass progression.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineBattlePassInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineBattlePassInterface
{
	GENERATED_BODY()

public:
	virtual bool CanAccessBattlePass(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
		return true;
	}

	virtual void ClaimBattlePass(const FAVVMPlayerChallenge& ChallengeContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullBattlePass(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};

/**
 *	Class description:
 *
 *	UAVVMOnlineStoreInterface is the subsystem interfacing with the microservice for the game store and the content that can be purchased outside gameplay.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineStoreInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineStoreInterface
{
	GENERATED_BODY()

public:
	virtual void SellItem(const FAVVMPlayerResource& ResourceContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void BuyItem(const FAVVMPlayerResource& ResourceContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullShopContent(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};
