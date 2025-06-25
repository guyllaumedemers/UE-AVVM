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
#include "Resources/AVVMResourceManagerComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "DataRegistrySubsystem.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Resources/AVVMResourceProvider.h"

// @gdemers extern symbol for global access to custom LLM_tag
AVVM_API extern FLLMTagDeclaration LLMTagDeclaration_AVVMTag;

// @gdemers for tracing nested resources loading request
TRACE_DECLARE_INT_COUNTER(UAVVMResourceManagerComponent_RequestCounter, TEXT("Resource Component Loading Request Counter"));
TRACE_DECLARE_INT_COUNTER(UAVVMResourceManagerComponent_InstanceCounter, TEXT("Resource Component Instance Counter"));

UAVVMResourceManagerComponent::FResourceQueueingMechanism::~FResourceQueueingMechanism()
{
	StreamableHandles.Reset();
	PendingRequests.Empty();
	CompletionDelegate.Clear();
}

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::TryExecuteNextRequest(const FOnAsyncLoadingRequestDeferred& NewRequest)
{
	QueueRequest(NewRequest);

	if (ensureAlwaysMsgf(HasPendingRequest(), TEXT("Queue should never be empty!")) && !HasUnfinishedStreamableHandle())
	{
		UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred PendingRequest;
		PendingRequests.Dequeue(PendingRequest);
		return PendingRequest.ExecuteIfBound();
	}
	else
	{
		return false;
	}
}

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::HasUnfinishedStreamableHandle() const
{
	return !StreamableHandles.IsEmpty() && StreamableHandles.Last()->IsLoadingInProgress();
}

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::HasPendingRequest() const
{
	return !PendingRequests.IsEmpty();
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::PushStreamableHandle(TSharedPtr<FStreamableHandle> NewStreamableHandle)
{
	StreamableHandles.AddUnique(NewStreamableHandle);
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::SetCompletionCallback(const FOnResourceAsyncLoadingComplete& NewRequestExternalCallback)
{
	CompletionDelegate = NewRequestExternalCallback;
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::GetLoadedAssets(TArray<UObject*>& OutStreamableAssets) const
{
	if (!StreamableHandles.IsEmpty())
	{
		StreamableHandles.Top()->GetLoadedAssets(OutStreamableAssets);
	}
}

const FOnResourceAsyncLoadingComplete& UAVVMResourceManagerComponent::FResourceQueueingMechanism::GetCompletionDelegate() const
{
	return CompletionDelegate;
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::QueueRequest(const UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred& NewRequest)
{
	PendingRequests.Enqueue(NewRequest);
}

void UAVVMResourceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	OwningOuter = Outer;

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UAVVMResourceManagerComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	LLM_SCOPE_BYTAG(AVVMTag);

#if WITH_SERVER_CODE
	if (bShouldAsyncLoadOnBeginPlay)
	{
		RequestAsyncLoading(IAVVMResourceProvider::Execute_GetResourceDefinitionResourceId(Outer), FOnResourceAsyncLoadingComplete{});
	}
#endif
}

void UAVVMResourceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UAVVMResourceManagerComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	LLM_SCOPE_BYTAG(AVVMTag);
}

void UAVVMResourceManagerComponent::RequestAsyncLoading(const FDataRegistryId& NewRegistryId,
                                                        const FOnResourceAsyncLoadingComplete& OnRequestCompleteCallback)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_RequestCounter);

	const auto OnDataAcquiredCallback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired, OnRequestCompleteCallback);
	ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(NewRegistryId, OnDataAcquiredCallback),
	                 TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
}

void UAVVMResourceManagerComponent::OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result,
                                                         FOnResourceAsyncLoadingComplete OnRequestCompleteCallback)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer Actor!")))
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	const bool bIsFullyLoaded = Result.Status == EDataRegistryAcquireStatus::AcquireFinished;
	if (!bIsFullyLoaded)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Resource Acquisition for \"%s\" Failed on Actor \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *Result.ItemId.ToString(),
		       *Outer->GetName());

		return;
	}

	const auto* DataTableRow = Result.GetItem<FAVVMDataTableRow>();
	check(DataTableRow != nullptr);

	const auto RequestSoftObjectsAsync = [&](const TArray<FSoftObjectPath>& ResourcePaths, const FOnResourceAsyncLoadingComplete& ActiveRequestCallback)
	{
		FStreamableDelegate CompletionCallback;
		CompletionCallback.BindUObject(this, &UAVVMResourceManagerComponent::OnSoftObjectAcquired);

		const TSharedPtr<FStreamableHandle> NewStreamableHandle = UAssetManager::Get().LoadAssetList(ResourcePaths, CompletionCallback);
		QueueingMechanism.SetCompletionCallback(ActiveRequestCallback);
		QueueingMechanism.PushStreamableHandle(NewStreamableHandle);
	};

	const auto WrappedDeferredRequest = FOnAsyncLoadingRequestDeferred::CreateWeakLambda(this, RequestSoftObjectsAsync, DataTableRow->GetResourcesPaths(), OnRequestCompleteCallback);
	const bool bWasRequestExecutedOrDeferred = QueueingMechanism.TryExecuteNextRequest(WrappedDeferredRequest);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Resource Acquisition Request for \"%s\" was \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Result.ItemId.ToString(),
	       bWasRequestExecutedOrDeferred ? TEXT("Executed") : TEXT("Deferred"));
}

void UAVVMResourceManagerComponent::OnSoftObjectAcquired()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer Actor!")))
	{
		QueueingMechanism.GetCompletionDelegate().ExecuteIfBound();
		return;
	}

	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IAVVMResourceProvider, UAVVMResourceProvider>(Outer);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IAVVMResourceProvider interface!")))
	{
		QueueingMechanism.GetCompletionDelegate().ExecuteIfBound();
		return;
	}

	TArray<UObject*> OutStreamedAssets;
	QueueingMechanism.GetLoadedAssets(OutStreamedAssets);

	FKeepProcessingResources KeepProcessingRegistriesCallback;
	KeepProcessingRegistriesCallback.BindDynamic(this, &UAVVMResourceManagerComponent::OnProcessAdditionalResources);

	const bool bIsDoneAcquiringResources = IAVVMResourceProvider::Execute_CheckIsDoneAcquiringResources(Outer, OutStreamedAssets, KeepProcessingRegistriesCallback);
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Is Resource Manager Done Acquiring Resources on Actor \"%s\"? %s"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName(),
	       bIsDoneAcquiringResources ? TEXT("True") : TEXT("False"));
}

bool UAVVMResourceManagerComponent::OnProcessAdditionalResources(const TArray<FDataRegistryId>& PendingRegistriesId)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		QueueingMechanism.GetCompletionDelegate().ExecuteIfBound();
		return false;
	}

	const bool bIsEmpty = PendingRegistriesId.IsEmpty();
	if (bIsEmpty)
	{
		QueueingMechanism.GetCompletionDelegate().ExecuteIfBound();
		return true;
	}

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_RequestCounter);

	for (const FDataRegistryId& RegistryId : PendingRegistriesId)
	{
		if (RegistryId.IsValid())
		{
			const auto CompletionCallback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired, QueueingMechanism.GetCompletionDelegate());
			ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(RegistryId, CompletionCallback), TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
		}
	}

	return false;
}
