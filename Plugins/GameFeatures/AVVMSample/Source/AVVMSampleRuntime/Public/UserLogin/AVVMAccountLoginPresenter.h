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
#include "Archetypes/AVVMPresenter.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMAccountLoginPresenter.generated.h"

enum class EAsyncWidgetLayerState : uint8;
class UCommonActivatableWidget;

/**
 *	Class description:
 *
 *	UAVVMAccountLoginPresenter handle user login request.
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMAccountLoginPresenter : public UAVVMPresenter
{
	GENERATED_BODY()

public:
	virtual AActor* GetOuterKey() const override;

protected:
	virtual void StartPresenting() override;
	virtual void StopPresenting() override;

	void OnPresenterStartCompleted(EAsyncWidgetLayerState State, UCommonActivatableWidget* ActivatableWidget);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);
	
	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	// @gdemers UCommonActivatableWidgetContainerBase handle memory lifetime for Actiavatable Widget.
	UPROPERTY(Transient)
	TWeakObjectPtr<UCommonActivatableWidget> PresentingWidget = nullptr;
};
