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
#include "StructUtils/InstancedStruct.h"

/**
 *	Class description:
 *
 *	FAVVMPlayerProfile define a user profile information.
 *
 *	example : Level, Money, Prestige, etc...
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMPlayerProfile : public FAVVMNotificationPayload
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	FAVVMParty define a group of person that a player can join.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMParty : public FAVVMNotificationPayload
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	FAVVMPlayerConnection encapsulate the player status when part of a party.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMPlayerConnection : public FAVVMNotificationPayload
{
	GENERATED_BODY()
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
	DECLARE_MULTICAST_DELEGATE_TwoParams(FAVVMOnlineResquestDelegate,
	                                     const bool /*bWasSuccess*/,
	                                     const TInstancedStruct<FAVVMNotificationPayload>& /*Payload*/);

	// @gdemers execute login request with backend
	virtual void RequestLogin(const FAVVMLoginContext& LoginContext, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	};

	// @gdemers commit local changes to backend services
	virtual void CommitModifiedPlayerProfile(const FAVVMPlayerProfile& PlayerContext, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}

	// @gdemers bonus function for when coming back from gameplay and having to pull the local player backend latest data!
	virtual void ForcePullPlayerProfile(const FAVVMPlayerProfile& PlayerContext, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}

	virtual void ConnectPlayerToParty(const FAVVMPlayerConnection& ConnectionContext, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}

	virtual void DisconnectPlayerFromParty(const FAVVMPlayerConnection& ConnectionContext, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}

	// @gdemers bonus function for when entering the game default map! our expectation is that whatever service we are tied to will
	// return us all available groups/entities that are "joinable" through user interaction.
	virtual void ForcePullParties(const TArray<FAVVMParty>& PartyContexts, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}
};
