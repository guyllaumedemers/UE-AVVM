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

#include "AVVMGameplayModule.h"
#include "AVVMGameplayUtils.h"
#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
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

UAVVMResourceManagerComponent::FNonTSResourceValidationMechanism::FNonTSResourceValidationMechanism()
	: NumRegistryIdRequested(0)
	, NumRegistryIdLoaded(0)
	, NumUObjectRequested(0)
	, NumUObjectLoaded(0)
{
}

void UAVVMResourceManagerComponent::FNonTSResourceValidationMechanism::IncrementRegistryIdRequested(FNonTSResourceValidationMechanism* Src)
{
	if (Src != nullptr)
	{
		++Src->NumRegistryIdRequested;
	}
}

void UAVVMResourceManagerComponent::FNonTSResourceValidationMechanism::IncrementRegistryIdLoaded(FNonTSResourceValidationMechanism* Src)
{
	if (Src != nullptr)
	{
		++Src->NumRegistryIdLoaded;
	}
}

void UAVVMResourceManagerComponent::FNonTSResourceValidationMechanism::IncrementUObjectRequested(FNonTSResourceValidationMechanism* Src,
                                                                                            const int32 NumResourcesRequested)
{
	if (Src != nullptr)
	{
		Src->NumUObjectRequested += NumResourcesRequested;
	}
}

void UAVVMResourceManagerComponent::FNonTSResourceValidationMechanism::IncrementUObjectLoaded(FNonTSResourceValidationMechanism* Src,
                                                                                         const int32 NumResourcesLoaded)
{
	if (Src != nullptr)
	{
		Src->NumUObjectLoaded += NumResourcesLoaded;
	}
}

bool UAVVMResourceManagerComponent::FNonTSResourceValidationMechanism::IsIntegral(FNonTSResourceValidationMechanism* Src)
{
	if (Src != nullptr)
	{
		return (Src->NumRegistryIdRequested > 0)
				&& (Src->NumUObjectRequested > 0)
				&& (Src->NumRegistryIdRequested == Src->NumRegistryIdLoaded)
				&& (Src->NumUObjectRequested == Src->NumUObjectLoaded);
	}
	else
	{
		return false;
	}
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

UAVVMResourceManagerComponent::UAVVMResourceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(false);
}

void UAVVMResourceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_AUTOMATION_TESTS
	NonTSValidationMechanism = MakeShared<FNonTSResourceValidationMechanism>();
#endif

	QueueingMechanism = MakeShared<FResourceQueueingMechanism>();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Missing Outer.")))
	{
		return;
	}

	OwningOuter = Outer;
	
	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_InstanceCounter);
	LLM_SCOPE_BYTAG(AVVMTag);
	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(UAVVMResourceManagerComponent::StaticClass()));

#if WITH_SERVER_CODE
	if (!bShouldAsyncLoadOnBeginPlay || !Outer->HasAuthority())
	{
		return;
	}

	const TArray<FDataRegistryId> ResourceIds = IAVVMResourceProvider::Execute_GetResourceDefinitionResourceIds(Outer);
	for (const auto& ResourceId : ResourceIds)
	{
		RequestAsyncLoading(ResourceId, FOnResourceAsyncLoadingComplete{});
	}
#endif
}

void UAVVMResourceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	NonTSValidationMechanism.Reset();
	QueueingMechanism.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Missing Outer.")))
	{
		return;
	}

	LLM_SCOPE_BYTAG(AVVMTag);
	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UAVVMResourceManagerComponent::StaticClass()));

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
	AVVM_LOGGER_LOG(LogGameplay,
	                OwningOuter.Get(),
	                OwningOuter.Get(),
	                TEXT("Requesting Resource Acquisition for %s."),
	                *NewRegistryId.ToString());

#if WITH_AUTOMATION_TESTS
	FNonTSResourceValidationMechanism::IncrementRegistryIdRequested(NonTSValidationMechanism.Get());
#endif

	const auto OnDataAcquiredCallback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired, OnRequestCompleteCallback);
	ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(NewRegistryId, OnDataAcquiredCallback), TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
}

#if WITH_AUTOMATION_TESTS
bool UAVVMResourceManagerComponent::AreResourcesIntegral() const
{
	return FNonTSResourceValidationMechanism::IsIntegral(NonTSValidationMechanism.Get());
}
#endif

void UAVVMResourceManagerComponent::OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result,
                                                         FOnResourceAsyncLoadingComplete OnRequestCompleteCallback)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	const bool bIsFullyLoaded = (Result.Status == EDataRegistryAcquireStatus::AcquireFinished);
	if (!bIsFullyLoaded)
	{
		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("Resource Acquisition for %s Failed."),
		                *Result.ItemId.ToString());

		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		OnRequestCompleteCallback.ExecuteIfBound();
		return;
	}

	const auto* DataTableRow = Result.GetItem<FAVVMDataTableRow>();
	if (!ensureAlwaysMsgf(DataTableRow != nullptr,
	                      TEXT("Resource loaded doesn't derive from %s."), *GetNameSafe(FAVVMDataTableRow::StaticStruct())))
	{
		return;
	}

#if WITH_AUTOMATION_TESTS
	FNonTSResourceValidationMechanism::IncrementRegistryIdLoaded(NonTSValidationMechanism.Get());
#endif

	const auto RequestSoftObjectsAsync = [](const TWeakObjectPtr<UAVVMResourceManagerComponent>& NewResourceManagerComponent,
	                                        TSharedPtr<FResourceQueueingMechanism> NewQueuingMechanism,
	                                        TSharedPtr<FNonTSResourceValidationMechanism> NewValidationMechanism,
	                                        const TArray<FSoftObjectPath>& ResourcePaths,
	                                        const FDataRegistryId& NewRegistryId,
	                                        const FOnResourceAsyncLoadingComplete& MainRequestCompletionCallback)
	{
		if (!ensureAlwaysMsgf(NewResourceManagerComponent.IsValid(), TEXT("WeakObjectPtr to Resource Manager Component Invalid!")) ||
			!ensureAlwaysMsgf(NewQueuingMechanism.IsValid(), TEXT("SharedPtr to Queueing Mechanism Invalid!")))
		{
			MainRequestCompletionCallback.ExecuteIfBound();
			return;
		}

		const AActor* NewOuter = NewResourceManagerComponent->OwningOuter.Get();
		AVVM_LOGGER_LOG(LogGameplay,
		                NewOuter,
		                NewOuter,
		                TEXT("Making call to UAssetManager to load resource object for %s."),
		                *NewRegistryId.ToString());

		FStreamableDelegate CompletionCallback;
		CompletionCallback.BindUObject(NewResourceManagerComponent.Get(), &UAVVMResourceManagerComponent::OnSoftObjectAcquired);

#if WITH_AUTOMATION_TESTS
		FNonTSResourceValidationMechanism::IncrementUObjectRequested(NewValidationMechanism.Get(), ResourcePaths.Num());
#endif

		const TSharedPtr<FStreamableHandle> NewStreamableHandle = UAssetManager::Get().LoadAssetList(ResourcePaths, CompletionCallback);
		NewQueuingMechanism->SetCompletionCallback(MainRequestCompletionCallback);
		NewQueuingMechanism->PushStreamableHandle(NewStreamableHandle);
	};

	const auto NewRequest = FOnAsyncLoadingRequestDeferred::CreateWeakLambda(this,
	                                                                         RequestSoftObjectsAsync,
	                                                                         this,
	                                                                         QueueingMechanism,
	                                                                         NonTSValidationMechanism,
	                                                                         DataTableRow->GetResourcesPaths(),
	                                                                         Result.ItemId,
	                                                                         OnRequestCompleteCallback);

	const bool bHasPendingRequest = QueueingMechanism->PushDeferredRequest(NewRequest);
	if (bHasPendingRequest)
	{
		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("UAssetManager resource acquisition request for %s was Deferred."),
		                *Result.ItemId.ToString());
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

	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMResourceProvider>(Outer);
	if (!bResult)
	{
		return;
	}

	TArray<UObject*> OutStreamedAssets;
	QueueingMechanism->GetLoadedAssets(OutStreamedAssets);
	QueueingMechanism->ModifyStreamableHandle();

#if WITH_AUTOMATION_TESTS
	FNonTSResourceValidationMechanism::IncrementUObjectLoaded(NonTSValidationMechanism.Get(), OutStreamedAssets.Num());
#endif

	// @gdemers recurse on nested registry id until our object is fully loaded.
	const TArray<FDataRegistryId> RecursiveResources = IAVVMResourceProvider::Execute_CheckIsDoneAcquiringResources(Outer, OutStreamedAssets);
	const bool bIsDoneAcquiringResources = OnProcessAdditionalResources(RecursiveResources);

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Is Resource Manager Done Acquiring Resources. %s"),
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
