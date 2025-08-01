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
#include "StructUtils/InstancedStruct.h"

#include "AVVMChallengeManagerPresenter.generated.h"

/**
*	Class description:
 *
 *	UAVVMChallengesPresenter define a set of objectives to be completed to earn rewards during gameplay. Can be inspected when in a
 *	serverless configuration. (via backend calls)
 *
 *	Note : For gameplay specific notification, like QTE notification showing updates, the expected notification source is via the
 *	UAVVMQuicktimeEventManagerPresenter.
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMChallengeManagerPresenter : public UAVVMPresenter,
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
	void BP_OnNotificationReceived_ForcePullChallenges(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_ClaimChallengeRewards(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void SetChallenges(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	virtual void StartPresenting() override;
	virtual void StopPresenting() override;
	virtual void BindViewModel() const override;

	void OnForcePullChallengesCompleted(const bool bWasSuccess,
	                                    const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	void OnClaimChallengeRewardsCompleted(const bool bWasSuccess,
	                                      const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRequestSuccess(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRequestFailure(const TInstancedStruct<FAVVMNotificationPayload>& Payload);
};
