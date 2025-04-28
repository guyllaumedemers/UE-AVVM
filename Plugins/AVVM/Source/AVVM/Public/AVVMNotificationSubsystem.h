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

class IAVVMObserver;

/**
 *	Class description:
 *
 *	FObserverContextArgs encapsulate arguments of UAVVMNotificationSubsystem api for better code readability.
 */
USTRUCT(BlueprintType)
struct AVVM_API FObserverContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsClassDefaultObject = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	UWorld* WorldContext = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TScriptInterface<IAVVMObserver> Observer = nullptr;
};

/**
 *	Class description:
 *
 *	FNotificationContextArgs encapsulate arguments of UAVVMNotificationSubsystem api for better code readability.
 */
USTRUCT(BlueprintType)
struct AVVM_API FNotificationContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	UWorld* WorldContext = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTag ChannelTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadOnly)
	FInstancedStruct Payload;
};

/**
*	Class description:
 *
 *	UAVVMNotificationSubsystem notify Presenters of a "Gameplay event" through Tag Channels. This system is expected to reduce
 *	the amount of boiler plate required for Presenter to listen to "Gameplay event".
 *
 *	Use UAVVMNotificationSubsystem::Static_BroadcastChannel in Game Code to broadcast to the expected Presenter UObject instead of
 *	having the Presenter fetch from it's Outer Actor Game content to listen for state change events.
 */
UCLASS()
class AVVM_API UAVVMNotificationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	inline static UAVVMNotificationSubsystem* Get(const UWorld* WorldContext);

	static void Static_BroadcastChannel(const FNotificationContextArgs& NotificationContext);
	static void Static_UnregisterObserver(const FObserverContextArgs& ObserverContext);
	static void Static_RegisterObserver(const FObserverContextArgs& ObserverContext);

protected:
	struct FTagChannelObserverCollection
	{
		~FTagChannelObserverCollection();

		void ResolveObservers(const FInstancedStruct& Payload, TArray<TScriptInterface<IAVVMObserver>>& Out) const;
		void RemoveOrDestroy(const TScriptInterface<IAVVMObserver>& Observer);
		void CreateOrAdd(const TScriptInterface<IAVVMObserver>& Observer);

	private:
		// @gdemers a collection of all observers that listen for a given channel.
		TArray<TScriptInterface<IAVVMObserver>> Observers;
	};

	void BroadcastChannel(const FInstancedStruct& Payload,
	                      const FGameplayTag& ChannelTag) const;

	void RemoveOrDestroy(const TScriptInterface<IAVVMObserver>& Observer,
	                     const FGameplayTagContainer& TagContainer);

	void CreateOrAdd(const TScriptInterface<IAVVMObserver>& Observer,
	                 const FGameplayTagContainer& TagContainer);

	// @gdemers a collection of all channels we can broadcast and notify observers with based on provided "Gameplay event".
	TMap<FGameplayTag, FTagChannelObserverCollection> TagChannels;
};
