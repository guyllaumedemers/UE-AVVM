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

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "DataRegistrySubsystem.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Resources/AVVMResourceImplementer.h"

void UAVVMResourceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* ResourceRequester = GetTypedOuter<AActor>();
	RequestExternalResourceAsync(ResourceRequester);
	OwningOuter = ResourceRequester;
}

void UAVVMResourceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ResourceHandles.Reset();
}

void UAVVMResourceManagerComponent::RequestExternalResourceAsync(const AActor* Outer)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		return;
	}

	const auto Callback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired);
	ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(IAVVMResourceImplementer::Execute_GetActorDefinitionResourceId(Outer), Callback),
	                 TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
}

void UAVVMResourceManagerComponent::OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result)
{
	const bool bIsFullyLoaded = Result.Status == EDataRegistryAcquireStatus::AcquireFinished;
	if (!bIsFullyLoaded)
	{
		UE_LOG(LogUI, Log, TEXT("Resource.Acquisition.%s. Failed!"), *Result.ItemId.ToString());
		return;
	}

	const auto* UnknownDataTableRow = Result.GetItem<FAVVMSandboxDataTableRow>();
	check(UnknownDataTableRow != nullptr);

	FStreamableDelegate Callback;
	Callback.BindUObject(this, &UAVVMResourceManagerComponent::OnSoftObjectAcquired);

	const TSharedPtr<FStreamableHandle> StreamableHandle = UAssetManager::Get().LoadAssetList(UnknownDataTableRow->GetResources(), Callback);
	ResourceHandles.Add(StreamableHandle);
}

void UAVVMResourceManagerComponent::OnSoftObjectAcquired()
{
	const auto ResourceImplementer = TScriptInterface<const IAVVMResourceImplementer>(OwningOuter.Get());
	if (!UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(ResourceImplementer))
	{
		return;
	}

	const auto TopHandle = !ResourceHandles.IsEmpty() ? ResourceHandles.Top() : TSharedPtr<FStreamableHandle>();

	TArray<const UObject*> OutStreamedAssets;
	TopHandle->GetLoadedAssets(OutStreamedAssets);

	FKeepProcessingResources Callback;
	Callback.BindDynamic(this, &UAVVMResourceManagerComponent::OnRegistriesPending);

	const bool bResult = ResourceImplementer->CheckIsDoneAcquiringResources(OutStreamedAssets, Callback);
	UE_LOG(LogUI,
	       Log,
	       TEXT("Resource Loader Update Status: Still Acquiring Resources ? %s"),
	       bResult ? TEXT("False") : TEXT("True"));
}

void UAVVMResourceManagerComponent::OnRegistriesPending(const TArray<FDataRegistryId>& PendingRegistriesId)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		return;
	}

	for (const FDataRegistryId& RegistryId : PendingRegistriesId)
	{
		const auto Callback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired);
		ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(RegistryId, Callback), TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
	}
}
