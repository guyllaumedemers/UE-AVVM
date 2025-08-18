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
#include "ActorItemProgressionComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AVVMGameplayUtils.h"
#include "InventoryProgressionSample.h"
#include "Data/ItemProgressionDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

TRACE_DECLARE_INT_COUNTER(UActorItemProgressionComponent_InstanceCounter, TEXT("Item Progression Component Instance Counter"));

void UActorItemProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UActorItemProgressionComponent_InstanceCounter);

	OwningOuter = Outer;

	UE_LOG(LogProgressionSample,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorItemProgressionComponent::StaticClass()->GetName(),
	       *Outer->GetName());
}

void UActorItemProgressionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ItemProgressionStagesHandle.Reset();
	ItemProgressionEffectsHandle.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogProgressionSample,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorItemProgressionComponent::StaticClass()->GetName(),
	       *Outer->GetName());

	auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Outer);
	if (IsValid(ASC))
	{
		for (auto Iterator = ActiveGameplayEffectHandles.CreateIterator(); Iterator; ++Iterator)
		{
			ASC->RemoveActiveGameplayEffect(*Iterator);
		}
	}

	ActiveGameplayEffectHandles.Reset();
}

void UActorItemProgressionComponent::SetProgressionIndex(const int32 NewProgressionIndex)
{
	CurrentProgressionStageIndex = NewProgressionIndex;
}

void UActorItemProgressionComponent::RequestItemProgression(const FDataRegistryId& NewItemProgressionId)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	auto* ResourceManagerComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(Outer);
	if (IsValid(ResourceManagerComponent))
	{
		ResourceManagerComponent->RequestAsyncLoading(NewItemProgressionId, {});
	}
}

void UActorItemProgressionComponent::SetupItemProgressionStages(const TArray<UObject*>& NewResources)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                      TEXT("Attempting to load invalid Item set on Outer \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	if (!ensureAlwaysMsgf(CurrentProgressionStageIndex != INDEX_NONE,
	                      TEXT("Invalid Progression Index. Index not set!")))
	{
		return;
	}

	const auto* ItemProgressionDefinition = Cast<UItemProgressionDefinitionDataAsset>(NewResources[0]);
	if (IsValid(ItemProgressionDefinition))
	{
		FStreamableDelegate OnRequestItemActorClassComplete;
		OnRequestItemActorClassComplete.BindUObject(this, &UActorItemProgressionComponent::OnItemProgressionStagesRetrieved);

		const TArray<FSoftObjectPath> ItemProgressionStages = ItemProgressionDefinition->GetProgressionStages(CurrentProgressionStageIndex);
		ItemProgressionStagesHandle = UAssetManager::Get().LoadAssetList(ItemProgressionStages, OnRequestItemActorClassComplete);
	}
}

void UActorItemProgressionComponent::OnItemProgressionStagesRetrieved()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	ItemProgressionStagesHandle->GetLoadedAssets(OutStreamableAssets);

	TArray<FSoftObjectPath> ProgressionStagesEffects;
	for (UObject* StreamableAsset : OutStreamableAssets)
	{
		auto* ItemProgressionStageAsset = Cast<UItemProgressionStageDefinitionDataAsset>(StreamableAsset);
		if (IsValid(ItemProgressionStageAsset))
		{
			ProgressionStagesEffects.Append(ItemProgressionStageAsset->GetProgressionStackingEffectSoftObjectPaths());
		}
	}

	FStreamableDelegate OnRequestItemActorClassComplete;
	OnRequestItemActorClassComplete.BindUObject(this, &UActorItemProgressionComponent::OnProgressionStackingEffectsRetrieved);

	ItemProgressionEffectsHandle = UAssetManager::Get().LoadAssetList(ProgressionStagesEffects, OnRequestItemActorClassComplete);
}

void UActorItemProgressionComponent::OnProgressionStackingEffectsRetrieved()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	ItemProgressionEffectsHandle->GetLoadedAssets(OutStreamableAssets);

	auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Outer);
	if (!IsValid(ASC))
	{
		return;
	}

	TArray<FSoftObjectPath> ProgressionStagesEffects;
	for (UObject* StreamableAsset : OutStreamableAssets)
	{
		UE_LOG(LogProgressionSample,
		       Log,
		       TEXT("Executed from \"%s\". Adding Progression Stacking GameplayEffect \"%s\" applying on Outer \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *StreamableAsset->GetName(),
		       IsValid(Outer) ? *Outer->GetName() : TEXT("Unknown"));

		const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(Cast<UClass>(StreamableAsset), nullptr, nullptr);
		ActiveGameplayEffectHandles.Add(ASC->BP_ApplyGameplayEffectSpecToSelf(GESpecHandle));
	}
}
