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
#include "Archetypes/AVVMPresenter.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMAccountLoginPresenter.generated.h"

enum class EAsyncWidgetLayerState : uint8;
class UCommonActivatableWidget;

/**
 *	Class description:
 *
 *	FAVVMUserAccount encapsulate data specific to your game representation of the user
 *	account.
 *
 *	example : Level, Money, Prestige, etc...
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMUserAccount
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	FAVVMLoginContextData encapsulate information about the user input when
 *	trying to login.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMLoginContextData : public FAVVMNotificationPayload
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
	virtual void RequestLogin(const FAVVMLoginContextData& ContextData, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	};

	// @gdemers submit local changes to backend services
	virtual void PushAccount(const TInstancedStruct<FAVVMUserAccount>& ContextData, FAVVMOnlineResquestDelegate Callback)
	{
		bool bCompletionStatus;
		FAVVMSampleRuntime::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}
};


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
	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_TryLogin(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	virtual void StartPresenting() override;
	virtual void StopPresenting() override;

	void OnPresenterStartCompleted(EAsyncWidgetLayerState State,
	                               UCommonActivatableWidget* ActivatableWidget);

	void OnLoginRequestCompleted(const bool bWasSuccess,
	                             const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	// @gdemers expect to make use of CommonUser UBlueprintAsyncActionBase
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnLoginRequestFailure(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	// @gdemers UCommonActivatableWidgetContainerBase handle memory lifetime for Actiavatable Widget.
	UPROPERTY(Transient)
	TWeakObjectPtr<UCommonActivatableWidget> PresentingWidget = nullptr;
};
