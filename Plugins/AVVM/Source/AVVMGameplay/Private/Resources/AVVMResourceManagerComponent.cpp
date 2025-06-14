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

void UAVVMResourceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UAVVMResourceManagerComponent to Actor \"%s\". IsLocallyControlled: %s."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName(),
	       UAVVMGameplayUtils::PrintIsLocallyControlled(Outer).GetData())

	LLM_SCOPE_BYTAG(AVVMTag);

	RequestExternalResourceAsync(Outer);
	OwningOuter = Outer;
}

void UAVVMResourceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		ResourceHandles.Empty();
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UAVVMResourceManagerComponent to Actor \"%s\". IsLocallyControlled: %s."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName(),
	       UAVVMGameplayUtils::PrintIsLocallyControlled(Outer).GetData())

	LLM_SCOPE_BYTAG(AVVMTag);

	ResourceHandles.Empty();
}

void UAVVMResourceManagerComponent::RequestExternalResourceAsync(const AActor* Outer)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_RequestCounter);

	const auto Callback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired);
	ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(IAVVMResourceProvider::Execute_GetResourceDefinitionResourceId(Outer), Callback),
	                 TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
}

void UAVVMResourceManagerComponent::OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result)
{
	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	const bool bIsFullyLoaded = Result.Status == EDataRegistryAcquireStatus::AcquireFinished;
	if (!bIsFullyLoaded)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Resource Acquisition for \"%s\" Failed on Actor \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
		       *Result.ItemId.ToString(),
		       *Outer->GetName());

		return;
	}

	const auto* DataTableRow = Result.GetItem<FAVVMDataTableRow>();
	check(DataTableRow != nullptr);

	FStreamableDelegate Callback;
	Callback.BindUObject(this, &UAVVMResourceManagerComponent::OnSoftObjectAcquired);

	const TSharedPtr<FStreamableHandle> StreamableHandle = UAssetManager::Get().LoadAssetList(DataTableRow->GetResourcesPaths(), Callback);
	ResourceHandles.Add(StreamableHandle);
}

void UAVVMResourceManagerComponent::OnSoftObjectAcquired()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IAVVMResourceProvider, UAVVMResourceProvider>(Outer);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IAVVMResourceProvider interface!")) || ResourceHandles.IsEmpty())
	{
		return;
	}

	TArray<UObject*> OutStreamedAssets;
	ResourceHandles.Top()->GetLoadedAssets(OutStreamedAssets);

	FKeepProcessingResources Callback;
	Callback.BindDynamic(this, &UAVVMResourceManagerComponent::ProcessAdditionalResources);

	const bool bHasPendingRegistries = IAVVMResourceProvider::Execute_CheckIsDoneAcquiringResources(Outer, OutStreamedAssets, Callback);
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Is Resource Manager Done Acquiring Resources on Actor \"%s\"? %s"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName(),
	       bHasPendingRegistries ? TEXT("False") : TEXT("True"));
}

bool UAVVMResourceManagerComponent::ProcessAdditionalResources(const TArray<FDataRegistryId>& PendingRegistriesId)
{
	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem) || PendingRegistriesId.IsEmpty())
	{
		return false;
	}

	TRACE_COUNTER_INCREMENT(UAVVMResourceManagerComponent_RequestCounter);

	for (const FDataRegistryId& RegistryId : PendingRegistriesId)
	{
		const auto Callback = FDataRegistryItemAcquiredCallback::CreateUObject(this, &UAVVMResourceManagerComponent::OnRegistryIdAcquired);
		ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(RegistryId, Callback), TEXT("Resource Acquisition Callback failed to schedule Completion Delegate!"));
	}

	return true;
}
