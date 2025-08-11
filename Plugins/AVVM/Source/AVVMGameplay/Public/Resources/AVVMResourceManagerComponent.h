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
#include "Containers/Queue.h"
#include "Engine/StreamableManager.h"

#include "AVVMResourceManagerComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnResourceAsyncLoadingComplete);

/**
 *	Class description:
 *
 *	UAVVMResourceManagerComponent is a system handling resource loading, through the implementation of IAVVMResourceImplementer::GetResourceDefinitionResourceId(), invoked during OnBeginPlay
 *	or via direct call made to UAVVMResourceManagerComponent::RequestAsyncLoading.
 *
 *	Any user request made must provide a valid FDataRegistryId and a Completion Delegate. The Component Outer Actor is expected to implement the required interface, i.e IAVVMResourceImplementer
 *	and override the following call IAVVMResourceProvider::CheckIsDoneAcquiringResources.
 *
 *	Important : CheckIsDoneAcquiringResources will be the central point in which your system will parse the status of all resources currently loading and update based on your system requirements.
 *	To allow flexibility, components added at Runtime via GFP_AddComponent<T> should retrieve a Delegate from their Outer Actor, bind to it, and be notified so they are able to gather nested FDataRegistryIds.
 *
 *	See UAVVMAbilitySystemComponent::SetupAbilities as example! (Impl. details done in BP on the BP_AVVM_PlayerState)
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

	struct FResourceQueueingMechanism
	{
		~FResourceQueueingMechanism();
		bool PushDeferredRequest(const UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred& NewRequest);
		bool TryExecuteNextRequest();
		void SetCompletionCallback(const FOnResourceAsyncLoadingComplete& NewRequestExternalCallback);
		void PushStreamableHandle(TSharedPtr<FStreamableHandle> NewStreamableHandle);
		void GetLoadedAssets(TArray<UObject*>& OutStreamableAssets) const;
		void ModifyStreamableHandle();
		const FOnResourceAsyncLoadingComplete& GetCompletionDelegate() const;

	protected:
		bool HasUnfinishedStreamableHandle() const;
		bool HasPendingRequest() const;

		TArray<TPair<TSharedPtr<FStreamableHandle>, bool/*IsDoneStreaming*/>> StreamableHandles;
		TQueue<UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred> PendingRequests;
		FOnResourceAsyncLoadingComplete CompletionDelegate;
	};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TSharedPtr<FResourceQueueingMechanism> QueueingMechanism;
};
