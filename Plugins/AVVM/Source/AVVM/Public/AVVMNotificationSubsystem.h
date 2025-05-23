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

#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"

#include "AVVMNotificationSubsystem.generated.h"

struct FAVVMNotificationPayload;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAVVMOnChannelNotifiedMulticastDelegate, const TInstancedStruct<FAVVMNotificationPayload>&, Payload);

DECLARE_DYNAMIC_DELEGATE_OneParam(FAVVMOnChannelNotifiedSingleCastDelegate, const TInstancedStruct<FAVVMNotificationPayload>&, Payload);

/**
 *	Class description:
 *
 *	FAVVMObserverContextArgs encapsulate arguments of UAVVMNotificationSubsystem api for better code readability.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMObserverContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	UObject* WorldContextObject = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag ChannelTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadWrite)
	FAVVMOnChannelNotifiedSingleCastDelegate Callback;
};

/**
 *	Class description:
 *
 *	FAVVMNotificationContextArgs encapsulate arguments of UAVVMNotificationSubsystem api for better code readability.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMNotificationContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	UObject* WorldContextObject = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag ChannelTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadWrite)
	TInstancedStruct<FAVVMNotificationPayload> Payload;
};

/**
 *	Class description:
 *
 *	FAVVMNotificationPayload. Base Class of a notification Payload.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMNotificationPayload
{
	GENERATED_BODY()
};

/**
*	Class description:
 *
 *	UAVVMNotificationSubsystem notify Presenters of a "Gameplay event" through Tag Channels. This system is expected to reduce
 *	the amount of boiler plate required for Presenter to listen to "Gameplay event".
 *
 *	Use UAVVMNotificationSubsystem::Static_BroadcastChannel in Game Code to broadcast to the expected Presenter UObject instead of
 *	having the Presenter fetch from it's Outer Actor and listen for state change events.
 *
 *	It's expected that the Channel Tags are being composed as follow when registering new Presenters.
 *
 *	example : ModuleName.ChannelName.PresenterClass.GameplayEventType
 */
UCLASS()
class AVVM_API UAVVMNotificationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UAVVMNotificationSubsystem* Get(const UWorld* WorldContext);

	UFUNCTION(BlueprintCallable, Category="AVVM|Subsytem")
	static void Static_UnregisterObserver(const FAVVMObserverContextArgs& ObserverContext);

	UFUNCTION(BlueprintCallable, Category="AVVM|Subsytem")
	static void Static_RegisterObserver(const FAVVMObserverContextArgs& ObserverContext);

	UFUNCTION(BlueprintCallable, Category="AVVM|Subsytem")
	static void Static_BroadcastChannel(const FAVVMNotificationContextArgs& NotificationContext);

protected:
	void BroadcastChannel(const FAVVMNotificationContextArgs& NotificationContext) const;
	void RemoveOrDestroy(const FGameplayTag& ChannelTag, const UObject* DelegateOwner);
	void CreateOrAdd(const FGameplayTag& ChannelTag, const FAVVMOnChannelNotifiedSingleCastDelegate& Callback);

	// @gdemers a collection of all channels we can broadcast and notify observers with based on provided "Gameplay event".
	TMap<const FGameplayTag, FAVVMOnChannelNotifiedMulticastDelegate> TagChannels;
};
