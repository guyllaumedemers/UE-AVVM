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
#pragma once

#include "CoreMinimal.h"

#include "AVVMNotificationSubsystem.h"
#include "AVVMPrimaryGameLayoutInterface.h"
#include "Archetypes/AVVMPresenter.h"
#include "AVVMOnlineInterface.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMPlayerProfilePresenter.generated.h"

/**
 *	Class description:
 *
 *	UAVVMPlayerProfilePresenter encapsulate information about the local player Account.
 *
 *	example : Loadout, Level, achievements, etc...
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMPlayerProfilePresenter : public UAVVMPresenter,
                                                          public IAVVMPrimaryGameLayoutInterface
{
	GENERATED_BODY()

public:
	virtual AActor* GetOuterKey() const override;

protected:
	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_CommitModifiedPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_ForcePullPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_ProcessPlayerRequest(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void SetPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	virtual void StartPresenting() override;
	virtual void StopPresenting() override;
	virtual void BindViewModel() const override;

	void OnCommitPlayerProfileCompleted(const bool bWasSuccess,
	                                    const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void OnForcePullPlayerProfileCompleted(const bool bWasSuccess,
	                                       const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void TryInvitePlayer(const FAVVMPlayerRequest& PlayerRequest);

	void OnInvitePlayerCompleted(const bool bWasSuccess,
	                             const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void TryBlockPlayer(const FAVVMPlayerRequest& PlayerRequest);

	void OnBlockPlayerCompleted(const bool bWasSuccess,
	                            const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void TryAddFriend(const FAVVMPlayerRequest& PlayerRequest);

	void OnAddFriendCompleted(const bool bWasSuccess,
	                          const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void TryRemoveFriend(const FAVVMPlayerRequest& PlayerRequest);

	void OnRemoveFriendCompleted(const bool bWasSuccess,
	                             const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void TryTrade(const FAVVMPlayerRequest& PlayerRequest);

	void OnTradeCompleted(const bool bWasSuccess,
	                      const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	// @gdemers expect the type erased payload to provide error context information
	// for the messaging system.
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRequestSuccess(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRequestFailure(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_InvitePlayer(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_BlockPlayer(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_AddFriend(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_RemoveFriend(const TInstancedStruct<FAVVMNotificationPayload>& Payload);
};
