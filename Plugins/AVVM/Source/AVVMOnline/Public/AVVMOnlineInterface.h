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
#pragma once

#include "CoreMinimal.h"

#include "AVVMNotificationSubsystem.h"
#include "AVVMOnline.h"
#include "Online/CoreOnlineFwd.h"
#include "UObject/Interface.h"

#include "AVVMOnlineInterface.generated.h"

struct FAVVMHostConfigurationProxy;
struct FAVVMPlayerLoginContext;
struct FAVVMPartyProxy;
struct FAVVMPlayerChallenge;
struct FAVVMPlayerProfileProxy;
struct FAVVMPlayerResource;
struct FAVVMPlayerRequest;

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

	FAVVMStringPayload() = default;
	FAVVMStringPayload(const FString& NewPayload);
	bool operator==(const FAVVMStringPayload& Rhs) const;

	// @gdemers used to prevent creation of an explicit type wrapping a collection of user defined structs.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload = FString();
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

	virtual void RequestLogin(const FAVVMPlayerLoginContext& LoginContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	};

	virtual void CommitModifiedPlayerProfile(const FAVVMPlayerProfileProxy& PlayerContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullPlayerProfile(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void CommitModifiedHostConfiguration(const FAVVMHostConfigurationProxy& ConfigurationContext, FAVVMOnlineResquestDelegate Callback)
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
	virtual void JoinParty(const FAVVMPartyProxy& PartyContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ExitParty(const FAVVMPartyProxy& PartyContext, FAVVMOnlineResquestDelegate Callback)
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
