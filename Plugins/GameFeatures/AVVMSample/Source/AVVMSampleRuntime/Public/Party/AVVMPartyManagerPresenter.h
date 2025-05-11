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

#include "AVVMPrimaryGameLayoutInterface.h"
#include "Archetypes/AVVMPresenter.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMPartyManagerPresenter.generated.h"

struct FAVVMPlayerRequest;

/**
 *	Class description:
 *
 *	UAVVMPartyManagerPresenter encapsulate information about the available groups that are hosted by the backend or party created
 *	by other players (from the active world).
 *
 *	example : players that are in a group before leaving town or private lan games, etc...
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMPartyManagerPresenter : public UAVVMPresenter,
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
	void BP_OnNotificationReceived_ForcePullParties(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_JoinParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_ExitParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_ProcessPlayerRequest(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void SetParties(const TInstancedStruct<FAVVMNotificationPayload>& Payload);
	void SetLocalParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	virtual void StartPresenting() override;
	virtual void StopPresenting() override;
	virtual void BindViewModel() const override;

	void OnForcePullPartiesCompleted(const bool bWasSuccess,
	                                 const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void OnPartyJoinRequestCompleted(const bool bWasSuccess,
	                                 const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void OnPartyExitRequestCompleted(const bool bWasSuccess,
	                                 const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void TryKickPlayer(const FAVVMPlayerRequest& PlayerRequest);

	void OnKickFromPartyRequestCompleted(const bool bWasSuccess,
	                                     const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRequestSuccess(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRequestFailure(const TInstancedStruct<FAVVMNotificationPayload>& Payload);
};
