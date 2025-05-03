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
#include "AVVMSampleRuntimeModule.h"

#include "AVVMOnlineInterface.generated.h"

/**
 *	Class description:
 *
 *	FAVVMStringPayload encapsulate a json representation for user defined types. When unfold, the
 *	FString returned from backend should resolve like so.
 *
 *	example : TArray<FAVVMParty> Parties = USomeUtilsApi::FromJson(InputStringPayload);
 *
 *	IMPORTANT : Online Subsystem service should manage forwarding already resolved types. By the time it reach this
 *	system, we should know our type.
 *
 *	Note : Exception being when we manage a collection of a known type!
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMStringPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	// @gdemers encapsulate a collection of user defined type.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
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
struct AVVMSAMPLERUNTIME_API FAVVMPlayerWallet : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerWallet& Rhs) const;

	// @gdemers Options[0] = {CurrencyId={}, Amount={}}
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
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
struct AVVMSAMPLERUNTIME_API FAVVMPlayerProfile : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerProfile& Rhs) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Gamertag = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Xp = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Wallet = FString();

	// @gdemers placeholder. conditional to your game if achievements are tracked.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Achievements = FString();

	// @gdemers placeholder. conditional to your game if presets of type are allowed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Presets = FString();
};

/**
 *	Class description:
 *
 *	FAVVMHostConfiguration define a configuration for a play session.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMHostConfiguration : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMHostConfiguration& Rhs) const;

	// @gdemers define the experience the players of a party will go through
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString GameplayGameMode = FString();

	// @gdemers define any other project specific information that should be run for setting up the gameplay
	// experience
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Options = FString();
};

/**
 *	Class description:
 *
 *	FAVVMRuntimeResource encapsulate any item that can be gathered by the user during gameplay or general
 *	information about the player - i.e health, stamina, etc...
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMRuntimeResource : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMRuntimeResource& Rhs) const;

	// @gdemers {ResourceId={}, Amount={}}
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Options = FString();
};

/**
 *	Class description:
 *
 *	FAVVMRuntimeChallenge encapsulate the progress of any challenge that are available to the player during gameplay.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMRuntimeChallenge : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMRuntimeChallenge& Rhs) const;

	// @gdemers {ChallengeId={}, ChallengeProgress={}, ChallengeGoal={}}
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
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
struct AVVMSAMPLERUNTIME_API FAVVMPlayerConnection : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerConnection& Rhs) const;

	// @gdemers backend profile as json
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString PlayerProfile = FString();

	// @gdemers transient state in which a player connection can be.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EAVVMPlayerStatus PlayerStatus;

	// @gdemers convert using FUniqueNetIdString::Create()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString UniqueNetId = FString();

	// @gdemers backend challenges as json (convert into a collection)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString RuntimeChallenges = FString();

	// @gdemers backend resources as json (convert into a collection)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString RuntimeResources = FString();
};

/**
 *	Class description:
 *
 *	FAVVMParty define a group of person that a player can join and information about how their session is configured.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMParty : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMParty& Rhs) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString PartyUniqueId = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString HostConfiguration = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
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
	Invite,
	Block,
	Kick,
	Mute,
	Censor,
};

inline const TCHAR* EnumToString(EAVVMPlayerRequestType State)
{
	switch (State)
	{
		case EAVVMPlayerRequestType::Invite:
			return TEXT("Invite");
		case EAVVMPlayerRequestType::Block:
			return TEXT("Block");
		case EAVVMPlayerRequestType::Kick:
			return TEXT("Kick");
		case EAVVMPlayerRequestType::Mute:
			return TEXT("Mute");
		case EAVVMPlayerRequestType::Censor:
			return TEXT("Censor");
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
struct AVVMSAMPLERUNTIME_API FAVVMPlayerRequest : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerRequest& Rhs) const;

	// @gdemers caller.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString SrcPlayerUniqueNetId = FString();

	// @gdemers receiver.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString DestPlayerUniqueNetId = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EAVVMPlayerRequestType RequestType;
};

/**
 *	Class description:
 *
 *	FAVVMLoginContext encapsulate information about the user input when
 *	trying to login.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMLoginContext : public FAVVMNotificationPayload
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
 *	UAVVMOnlineInterface abstract project backend microservice.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMSAMPLERUNTIME_API UAVVMOnlineInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMSAMPLERUNTIME_API IAVVMOnlineInterface
{
	GENERATED_BODY()

public:
	virtual bool IsHosting(const FUniqueNetIdPtr PlayerUniqueNetIdPtr) const
	{
		return true;
	}

	virtual void RequestLogin(const FAVVMLoginContext& LoginContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	};

	virtual void CommitModifiedPlayerProfile(const FAVVMPlayerProfile& PlayerContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullPlayerProfile(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void JoinParty(const FAVVMParty& PartyContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ExitParty(const FAVVMParty& PartyContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void KickFromParty(const FString& UniqueNetId, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void InviteInParty(const FString& SrcUniqueNetId, const FString& DestUniqueNetId, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullParties(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void CommitModifiedHostConfiguration(const FAVVMHostConfiguration& ConfigurationContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullHostConfiguration(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ClaimChallenge(const FAVVMRuntimeChallenge& ChallengeContext, FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}

	virtual void ForcePullChallenges(FAVVMOnlineResquestDelegate Callback)
	{
		AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback);
	}
};
