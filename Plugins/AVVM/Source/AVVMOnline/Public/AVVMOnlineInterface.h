﻿//Copyright(c) 2025 gdemers
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

	// @gdemers encapsulate a collection of user defined type.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload = FString();
};

// ---------------------------------------------------------------------------------------------------------------------//
//							SomeUtils::To/FromJson api should be define for most Struct type here						//
// ---------------------------------------------------------------------------------------------------------------------//

/*
 *	Class description:
 *
 *	FAVVMPlayerWallet encapsulate currencies that your profile own. Crypto, event coupons,
 *	coins, etc...
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerWallet : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerWallet& Rhs) const;

	// @gdemers Options[0] = {CurrencyId={}, Amount={}}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Options;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerProfile define a user profile information.
 *
 *	example : profile name, xp, wallet, etc...
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerProfile : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerProfile& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Gamertag = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Xp = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Wallet = FString();

	// @gdemers placeholder. conditional to your game if achievements are tracked.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Achievements = FString();

	// @gdemers placeholder. conditional to your game if presets of type are allowed.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Presets = FString();
};

/**
 *	Class description:
 *
 *	FAVVMHostConfiguration define a configuration for a play session.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMHostConfiguration : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMHostConfiguration& Rhs) const;

	// @gdemers define the experience the players of a party will go through
	UPROPERTY(Transient, BlueprintReadWrite)
	FString GameplayGameMode = FString();

	// @gdemers define any other project specific information that should be run for setting up the gameplay
	// experience
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Options = FString();
};

/**
 *	Class description:
 *
 *	FAVVMRuntimeResource encapsulate any item that can be gathered by the user during gameplay or general
 *	information about the player - i.e health, stamina, etc...
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMRuntimeResource : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMRuntimeResource& Rhs) const;

	// @gdemers {ResourceId={}, Amount={}}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Options = FString();
};

/**
 *	Class description:
 *
 *	FAVVMRuntimeChallenge encapsulate the progress of any challenge that are available to the player during gameplay.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMRuntimeChallenge : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMRuntimeChallenge& Rhs) const;

	// @gdemers {ChallengeId={}, ChallengeProgress={}, ChallengeGoal={}}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Options = FString();
};

/**
 *	Class description:
 *
 *	EAVVMPlayerStatus define a set of possible state our player can be in.
 */
UENUM(BlueprintType)
enum class EAVVMPlayerStatus : uint8
{
	Default,
	Ready,
	PendingAction
};

/**
 *	Class description:
 *
 *	FAVVMPlayerConnection encapsulate the player status when part of a party.
 *
 *	example : information about the visual representation of the player.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerConnection : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerConnection& Rhs) const;

	// @gdemers backend profile as json
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PlayerProfile = FString();

	// @gdemers transient state in which a player connection can be.
	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMPlayerStatus PlayerStatus = EAVVMPlayerStatus::Default;

	// @gdemers convert using FUniqueNetIdString::Create()
	UPROPERTY(Transient, BlueprintReadWrite)
	FString UniqueNetId = FString();

	// @gdemers backend challenges as json (convert into a collection)
	UPROPERTY(Transient, BlueprintReadWrite)
	FString RuntimeChallenges = FString();

	// @gdemers backend resources as json (convert into a collection)
	UPROPERTY(Transient, BlueprintReadWrite)
	FString RuntimeResources = FString();
};

/**
 *	Class description:
 *
 *	FAVVMParty define a group of person that a player can join and information about how their session is configured.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMParty : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMParty& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString PartyUniqueId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString HostConfiguration = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString PlayerConnections = FString();
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
	virtual void ClaimChallenge(const FAVVMRuntimeChallenge& ChallengeContext, FAVVMOnlineResquestDelegate Callback)
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

	virtual void ClaimBattlePass(const FAVVMRuntimeChallenge& ChallengeContext, FAVVMOnlineResquestDelegate Callback)
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
	virtual void SellItem(const FAVVMRuntimeResource& ResourceContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void BuyItem(const FAVVMRuntimeResource& ResourceContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullShopContent(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}
};
