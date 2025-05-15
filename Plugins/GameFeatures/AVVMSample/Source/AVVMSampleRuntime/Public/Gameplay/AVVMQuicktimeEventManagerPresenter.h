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
#include "AVVMUIExtensionInterface.h"
#include "UIExtensionSystem.h"
#include "Archetypes/AVVMPresenter.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMQuicktimeEventManagerPresenter.generated.h"

/**
 *	Class description:
 *
 *	UAVVMQuicktimeEventManagerPresenter handle any "Fire-n-Forget" events triggered from Gameplay or Backend.
 *
 *	Note : More than a single FUIExtensionHandle will be cached on this system. Quicktime notification are fire-n-forget
 *	events which are invoked from a variadic number of systems.
 *
 *	FUIExtensionHandle should be discard once the fire-n-forget animation for a given QTE is complete. TBD!
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMQuicktimeEventManagerPresenter : public UAVVMPresenter,
                                                                  public IAVVMUIExtensionInterface
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
	void BP_OnNotificationReceived_RemoteQuicktimeEventFired(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	virtual void StartPresenting() override;
	virtual void StopPresenting() override;

	void ClearHandles();

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<FGameplayTag, FUIExtensionHandle> ExtensionHandles;
};
