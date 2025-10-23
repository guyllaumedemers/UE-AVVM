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
#include "GameFramework/Actor.h"
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

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::PushDeferredRequest(const FOnAsyncLoadingRequestDeferred& NewRequest)
{
	PendingRequests.Enqueue(NewRequest);
	TryExecuteNextRequest();
	return HasPendingRequest();
}

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::TryExecuteNextRequest()
{
	if (!HasPendingRequest())
	{
		CompletionDelegate.ExecuteIfBound();
		return true;
	}

	if (!HasUnfinishedStreamableHandle())
	{
		UAVVMResourceManagerComponent::FOnAsyncLoadingRequestDeferred NextRequest;
		PendingRequests.Dequeue(NextRequest);
		NextRequest.ExecuteIfBound();
	}

	return false;
}

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::HasUnfinishedStreamableHandle() const
{
	if (!StreamableHandles.IsEmpty())
	{
		const TPair<TSharedPtr<FStreamableHandle>, bool>& Last = StreamableHandles.Last();
		return !Last.Value/*IsDoneStreaming*/;
	}

	return false;
}

bool UAVVMResourceManagerComponent::FResourceQueueingMechanism::HasPendingRequest() const
{
	return !PendingRequests.IsEmpty();
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::PushStreamableHandle(TSharedPtr<FStreamableHandle> NewStreamableHandle)
{
	StreamableHandles.Add({NewStreamableHandle, false/*IsDoneStreaming*/});
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::SetCompletionCallback(const FOnResourceAsyncLoadingComplete& NewRequestExternalCallback)
{
	CompletionDelegate = NewRequestExternalCallback;
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::GetLoadedAssets(TArray<UObject*>& OutStreamableAssets) const
{
	if (StreamableHandles.IsEmpty())
	{
		return;
	}

	TSharedPtr<FStreamableHandle> StreamableHandle = StreamableHandles.Last().Key;
	if (StreamableHandle.IsValid())
	{
		StreamableHandle->GetLoadedAssets(OutStreamableAssets);
	}
}

void UAVVMResourceManagerComponent::FResourceQueueingMechanism::ModifyStreamableHandle()
{
	if (!StreamableHandles.IsEmpty())
	{
		TPair<TSharedPtr<FStreamableHandle>, bool>& Last = StreamableHandles.Last();
		Last.Value = true;
	}
}

const FOnResourceAsyncLoadingComplete& UAVVMResourceManagerComponent::FResourceQueueingMechanism::GetCompletionDelegate() const
{
	return CompletionDelegate;
}

void UAVVMResourceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	QueueingMechanism = MakeShared<FResourceQueueingMechanism>();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	OwningOuter = Outer;

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UAVVMResourceManagerComponent on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	LLM_SCOPE_BYTAG(AVVMTag);

#if WITH_SERVER_CODE
	if (bShouldAsyncLoadOnBeginPlay && Outer->HasAuthority())
	{
		const TArray<FDataRegistryId> ResourceIds = IAVVMResourceProvider::Execute_GetResourceDefinitionResourceIds(Outer);
		for (const auto& ResourceId : ResourceIds)
		{
			UE_LOG(LogGameplay,
			       Log,
			       TEXT("Executed from \"%s\". Requesting Resource Acquisition for \"%s\" on Outer \"%s\"!"),
			       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
			       *ResourceId.ToString(),
			       *Outer->GetName());

			RequestAsyncLoading(ResourceId, FOnResourceAsyncLoadingComplete{});
		}
	}
#endif
}

void UAVVMResourceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	QueueingMechanism.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UAVVMResourceManagerComponent on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	LLM_SCOPE_BYTAG(AVVMTag);

	OwningOuter.Reset();
}

void UAVVMResourceManagerComponent::RequestAsyncLoading(const FDataRegistryId& NewRegistryId,
                                                        const FOnResourceAsyncLoadingComplete& OnRequestCompleteCallback)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem) || !NewRegistryId.IsValid())
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_RequestCounter);

	const auto OnDataAcquiredCallback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired, OnRequestCompleteCallback);
	ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(NewRegistryId, OnDataAcquiredCallback), TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
}

void UAVVMResourceManagerComponent::OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result,
                                                         FOnResourceAsyncLoadingComplete OnRequestCompleteCallback)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	const bool bIsFullyLoaded = Result.Status == EDataRegistryAcquireStatus::AcquireFinished;
	if (!bIsFullyLoaded)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Resource Acquisition for \"%s\" Failed on Outer \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *Result.ItemId.ToString(),
		       *Outer->GetName());

		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	const auto* DataTableRow = Result.GetItem<FAVVMDataTableRow>();
	check(DataTableRow != nullptr);

	const auto RequestSoftObjectsAsync = [](TWeakObjectPtr<UAVVMResourceManagerComponent> NewResourceManagerComponent,
	                                        TSharedPtr<FResourceQueueingMechanism> NewQueuingMechanism,
	                                        TArray<FSoftObjectPath> ResourcePaths,
	                                        FDataRegistryId NewRegistryId,
	                                        FOnResourceAsyncLoadingComplete MainRequestCompletionCallback)
	{
		if (!ensureAlwaysMsgf(NewResourceManagerComponent.IsValid(), TEXT("WeakObjectPtr to Resource Manager Component Invalid!")) ||
			!ensureAlwaysMsgf(NewQueuingMechanism.IsValid(), TEXT("SharedPtr to Queueing Mechanism Invalid!")))
		{
			MainRequestCompletionCallback.ExecuteIfBound();
			return;
		}

		const AActor* NewOuter = NewResourceManagerComponent->OwningOuter.Get();
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Executing Resource Acquisition Request for \"%s\" on Outer \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(NewOuter).GetData(),
		       *NewRegistryId.ToString(),
		       IsValid(NewOuter) ? *NewOuter->GetName() : TEXT("Unknown"));

		FStreamableDelegate CompletionCallback;
		CompletionCallback.BindUObject(NewResourceManagerComponent.Get(), &UAVVMResourceManagerComponent::OnSoftObjectAcquired);

		const TSharedPtr<FStreamableHandle> NewStreamableHandle = UAssetManager::Get().LoadAssetList(ResourcePaths, CompletionCallback);
		NewQueuingMechanism->SetCompletionCallback(MainRequestCompletionCallback);
		NewQueuingMechanism->PushStreamableHandle(NewStreamableHandle);
	};

	const auto NewRequest = FOnAsyncLoadingRequestDeferred::CreateWeakLambda(this,
	                                                                         RequestSoftObjectsAsync,
	                                                                         TWeakObjectPtr<UAVVMResourceManagerComponent>(this),
	                                                                         QueueingMechanism,
	                                                                         DataTableRow->GetResourcesPaths(),
	                                                                         Result.ItemId,
	                                                                         OnRequestCompleteCallback);

	const bool bHasPendingRequest = QueueingMechanism->PushDeferredRequest(NewRequest);
	if (bHasPendingRequest)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Resource Acquisition Request for \"%s\" was Deferred on Outer \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *Result.ItemId.ToString(),
		       *Outer->GetName());
	}
}
 
void UAVVMResourceManagerComponent::OnSoftObjectAcquired()
{
	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const bool bResult = UAVVMUtilityFunctionLibrary::IsBlueprintScriptInterfaceValid<UAVVMResourceProvider>(Outer);
	if (!bResult)
	{
		return;
	}

	TArray<UObject*> OutStreamedAssets;
	QueueingMechanism->GetLoadedAssets(OutStreamedAssets);
	QueueingMechanism->ModifyStreamableHandle();

	// @gdemers recurse on nested registry id until our object is fully loaded.
	const TArray<FDataRegistryId> RecursiveResources = IAVVMResourceProvider::Execute_CheckIsDoneAcquiringResources(Outer, OutStreamedAssets);
	const bool bIsDoneAcquiringResources = OnProcessAdditionalResources(RecursiveResources);
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Is Resource Manager Done Acquiring Resources on Outer \"%s\"? %s"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName(),
	       bIsDoneAcquiringResources ? TEXT("True") : TEXT("False"));
}

bool UAVVMResourceManagerComponent::OnProcessAdditionalResources(const TArray<FDataRegistryId>& PendingRegistriesId)
{
	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		return false;
	}

	const bool bIsEmpty = PendingRegistriesId.IsEmpty();
	if (bIsEmpty)
	{
		return QueueingMechanism->TryExecuteNextRequest();
	}

	for (const FDataRegistryId& RegistryId : PendingRegistriesId)
	{
		RequestAsyncLoading(RegistryId, QueueingMechanism->GetCompletionDelegate());
	}

	return false;
}
