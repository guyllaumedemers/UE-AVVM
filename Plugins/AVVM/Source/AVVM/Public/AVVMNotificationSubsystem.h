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

#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"

#include "AVVMNotificationSubsystem.generated.h"

struct FAVVMNotificationPayload;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAVVMOnChannelNotifiedMulticastDelegate, const TInstancedStruct<FAVVMNotificationPayload>&, Payload);

DECLARE_DYNAMIC_DELEGATE_OneParam(FAVVMOnChannelNotifiedSingleCastDelegate, const TInstancedStruct<FAVVMNotificationPayload>&, Payload);

// @gdemers allow stripping symbols when building server target for dedicated server
#ifdef UE_AVVM_RUNNING_DEDICATED_SERVER
#define UE_AVVM_NOTIFY(WorldContextObject, ChannelTag, Target, Payload)
#define UE_AVVM_NOTIFY_IF_LOCALLYCONTROLLED(WorldContextObject, ChannelTag, LocallyControlledActor, ActorBoundToChannel, Payload)
#else
#define UE_AVVM_NOTIFY(WorldContextObject, ChannelTag, ActorBoundToChannel, Payload)\
	UAVVMNotificationSubsystem::Static_BroadcastChannel(WorldContextObject, FAVVMNotificationContextArgs{ChannelTag, ActorBoundToChannel, Payload});
#define UE_AVVM_NOTIFY_IF_LOCALLYCONTROLLED(WorldContextObject, ChannelTag, LocallyControlledActor, ActorBoundToChannel, Payload)\
	if(UAVVMGameplayUtils::IsLocallyControlled(LocallyControlledActor)) { UAVVMNotificationSubsystem::Static_BroadcastChannel(WorldContextObject, FAVVMNotificationContextArgs{ChannelTag, ActorBoundToChannel, Payload}); }
#endif

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

	UPROPERTY(Transient, BlueprintReadWrite, meta=(ToolTip="Target Channel. May have multiple Presenters listening."))
	FGameplayTag ChannelTag = FGameplayTag::EmptyTag;

	// @gdemers if no target is provided, we broadcast to all observers of the channel tag.
	UPROPERTY(Transient, BlueprintReadWrite, meta=(ToolTip="Unique Owner of the Presenter we are targeting."))
	const AActor* Target = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	TInstancedStruct<FAVVMNotificationPayload> Payload;
};

/**
 *	Class description:
 *
 *	FAVVMNotificationPayload is base class for a notification Payload.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMNotificationPayload
{
	GENERATED_BODY()

	static TInstancedStruct<FAVVMNotificationPayload> Empty;
};

/**
*	Class description:
 *
 *	UAVVMNotificationSubsystem is a system that register/unregister Observers (i.e Presenter Objects) and notify them of a "Gameplay event" through Tag Channels. This system allow filtering
 *	of Actors so we can notify for a unique target or to all targets listening to the channel.
 *
 *	Here's an example of the expected tag format :
 *
 *		* ModuleName.ChannelName.PresenterClass.GameplayEventType
 */
UCLASS()
class AVVM_API UAVVMNotificationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UAVVMNotificationSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="AVVM|Subsytem", meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static void Static_UnregisterObserver(const UObject* WorldContextObject,
	                                      const FAVVMObserverContextArgs& ObserverContext);

	UFUNCTION(BlueprintCallable, Category="AVVM|Subsytem", meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static void Static_RegisterObserver(const UObject* WorldContextObject,
	                                    const FAVVMObserverContextArgs& ObserverContext);

	UFUNCTION(BlueprintCallable, Category="AVVM|Subsytem", meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static void Static_BroadcastChannel(const UObject* WorldContextObject,
	                                    const FAVVMNotificationContextArgs& NotificationContext);

protected:
	struct FAVVMObservers
	{
		~FAVVMObservers();
		void Unregister(const AActor* Target);
		void Register(const AActor* Target, const FAVVMOnChannelNotifiedSingleCastDelegate& Callback);
		void BroadcastAll(const TInstancedStruct<FAVVMNotificationPayload>& Payload) const;
		void Broadcast(const AActor* Target, const TInstancedStruct<FAVVMNotificationPayload>& Payload) const;

		TMap<TWeakObjectPtr<const AActor>, FAVVMOnChannelNotifiedSingleCastDelegate> Observers;
	};

	struct FAVVObserversFilteringMechanism
	{
		~FAVVObserversFilteringMechanism();
		void Unregister(const AActor* Target, const FGameplayTag& ChannelTag);
		void Register(const AActor* Target, const FGameplayTag& ChannelTag, const FAVVMOnChannelNotifiedSingleCastDelegate& Callback);
		void Broadcast(const FAVVMNotificationContextArgs& NotificationContext) const;

		TMap<const FGameplayTag, FAVVMObservers> TagToObservers;
	};

	FAVVObserversFilteringMechanism ObserversFilteringMechanism;
};
