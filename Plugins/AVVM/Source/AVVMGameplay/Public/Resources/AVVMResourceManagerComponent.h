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

#include "DataRegistryId.h"
#include "DataRegistryTypes.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"

#include "AVVMResourceManagerComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnResourceAsyncLoadingComplete);

/**
 *	Class description:
 *
 *	UAVVMResourceManagerComponent dynamic component handling resource loading on any Actor that implement the required interface
 *	IAVVMResourceImplementer.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMResourceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DELEGATE(FOnAsyncLoadingRequestDeferred);

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void RequestAsyncLoading(const FDataRegistryId& NewRegistryId,
	                         const FOnResourceAsyncLoadingComplete& OnRequestCompleteCallback);

protected:
	void OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result,
	                          FOnResourceAsyncLoadingComplete OnRequestCompleteCallback);

	void OnSoftObjectAcquired();

	UFUNCTION()
	bool OnProcessAdditionalResources(const TArray<FDataRegistryId>& PendingRegistriesId);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldAsyncLoadOnBeginPlay = true;

	struct FResourceQueueingMechanism
	{
		virtual ~FResourceQueueingMechanism();
		bool TryExecuteNextRequest(const UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred& NewRequest);
		void SetCompletionCallback(const FOnResourceAsyncLoadingComplete& NewRequestExternalCallback);
		void PushStreamableHandle(TSharedPtr<FStreamableHandle> NewStreamableHandle);
		void GetLoadedAssets(TArray<UObject*>& OutStreamableAssets) const;
		const FOnResourceAsyncLoadingComplete& GetCompletionDelegate() const;

	protected:
		bool HasUnfinishedStreamableHandle() const;
		bool HasPendingRequest() const;
		void QueueRequest(const FOnAsyncLoadingRequestDeferred& NewRequest);

		TArray<TSharedPtr<FStreamableHandle>> StreamableHandles;
		TQueue<UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred> PendingRequests;
		FOnResourceAsyncLoadingComplete CompletionDelegate;
	};

	FResourceQueueingMechanism QueueingMechanism;
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
