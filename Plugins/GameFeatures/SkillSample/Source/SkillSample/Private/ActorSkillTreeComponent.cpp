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
#include "ActorSkillTreeComponent.h"

#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
#include "SkillSampleModule.h"
#include "SkillTreeNodeObject.h"
#include "SkillTreeProvider.h"
#include "SkillTreeUtils.h"
#include "Data/SkillTreeDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

TRACE_DECLARE_INT_COUNTER(UActorSkillTreeComponent_InstanceCounter, TEXT("SkillTree Component Instance Counter"));

TArray<int32> FSkillTreeDataResolverHelper::GetElementDependencies(const UObject* Outer, const int32 ElementId) const
{
	return FAVVMDataResolverHelper::GetElementDependencies(Outer, ElementId);
}

UActorSkillTreeComponent::UActorSkillTreeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = true;
}

void UActorSkillTreeComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_AutonomousOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(UActorSkillTreeComponent, SkillTreeNodes, Params);
}

void UActorSkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UActorSkillTreeComponent_InstanceCounter);
	AVVM_LOGGER_LOG(LogSkillSample,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(UActorSkillTreeComponent::StaticClass()));

	OwningOuter = Outer;

#if WITH_SERVER_CODE
	if (bShouldAsyncLoadOnBeginPlay && Outer->HasAuthority())
	{
		RequestSkillTree(Outer);
	}
#endif
}

void UActorSkillTreeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// @gdemers enforce cancelling running async process during actor destruction.
	for (auto Iterator = SkillTreeNodeHandleSystem.CreateIterator(); Iterator; ++Iterator)
	{
		const TSharedPtr<FStreamableHandle>& Tuple = Iterator->Value;
		if (Tuple.IsValid())
		{
			Tuple->CancelHandle();
		}
	}

	SkillTreeNodeHandleSystem.Reset();
	PrivateSkillTreeNodeIds.Reset();

	for (auto Iterator = SkillTreeNodes.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogSkillSample,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UActorSkillTreeComponent::StaticClass()));

	OwningOuter.Reset();
}

UActorSkillTreeComponent* UActorSkillTreeComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UActorSkillTreeComponent>() : nullptr;
}

void UActorSkillTreeComponent::RequestSkillTree(const AActor* Outer)
{
	if (!IsValid(Outer) || !ensureAlwaysMsgf(Outer->HasAuthority(),
	                                         TEXT("Outer isn't Authoritative and cannot request loading items!")))
	{
		return;
	}

	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<USkillTreeProvider>(Outer);
	if (!bResult)
	{
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(UActorSkillTreeComponent, SkillTreeNodes, this);
	for (auto Iterator = SkillTreeNodes.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	ESkillTreeSrcType OutSrcType = ESkillTreeSrcType::None;
	const bool bIsValid = USkillTreeUtils::GetOuterSourceType(Outer, OutSrcType);
	if (!bIsValid)
	{
		return;
	}

	AVVM_LOGGER_LOG(LogSkillSample,
	                Outer,
	                Outer,
	                TEXT("Requesting item type %s."),
	                EnumToString(OutSrcType));

	const bool bIsItemSrcStatic = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::Static);
	if (bIsItemSrcStatic)
	{
		ISkillTreeProvider::Execute_RequestItemsFromDataAsset(Outer);
	}
	else
	{
		ISkillTreeProvider::Execute_RequestItemsFromMicroService(Outer);
	}
}

void UActorSkillTreeComponent::SetupSkillTreeNodeObjects(const TArray<UObject*>& NewResources)
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

	TArray<FSoftObjectPath> DeferredItems;
	for (const UObject* Resource : NewResources)
	{
		const auto* SkillTreeNodeAsset = Cast<USkillTreeNodeDefinitionDataAsset>(Resource);
		if (!IsValid(SkillTreeNodeAsset))
		{
			continue;
		}

		if (!SkillTreeNodeAsset->CanAccessSkillTreeNodeObject(NonReplicatedComponentStateTags, NonReplicatedComponentStateTags))
		{
			AVVM_LOGGER_LOG(LogSkillSample,
			                Outer,
			                Outer,
			                TEXT("Failed to meet %s Requirements."),
			                *GetNameSafe(SkillTreeNodeAsset));

			continue;
		}

		DeferredItems.Add(SkillTreeNodeAsset->GetSkillTreeNodeObjectClass().ToSoftObjectPath());
		AVVM_LOGGER_LOG(LogSkillSample,
		                Outer,
		                Outer,
		                TEXT("%s pending request queued."),
		                *GetNameSafe(SkillTreeNodeAsset));
	}

	if (DeferredItems.IsEmpty())
	{
		return;
	}

	const auto Token = FSkillTreeNodeToken::MakeToken();

	FStreamableDelegate Callback;
	Callback.BindUObject(this, &UActorSkillTreeComponent::OnSkillTreeNodeRetrieved, Token);

	TSharedPtr<FStreamableHandle>& OutResult = SkillTreeNodeHandleSystem.FindOrAdd(Token.UniqueId);
	OutResult = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
}

const TInstancedStruct<FAVVMDataResolverHelper>& UActorSkillTreeComponent::GetSkillTreeDataResolverHelper()
{
	static auto Helper = FAVVMDataResolverHelper::Make<FSkillTreeDataResolverHelper>();
	return Helper;
}

void UActorSkillTreeComponent::OnSkillTreeNodeRetrieved(FSkillTreeNodeToken SkillTreeNodeToken)
{
	const TSharedPtr<FStreamableHandle>* OutResult = SkillTreeNodeHandleSystem.Find(SkillTreeNodeToken.UniqueId);
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	ESkillTreeSrcType OutSrcType = ESkillTreeSrcType::None;
	const bool bIsValid = USkillTreeUtils::GetOuterSourceType(Outer, OutSrcType);
	if (!bIsValid)
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	(*OutResult)->GetLoadedAssets(OutStreamableAssets);

	MARK_PROPERTY_DIRTY_FROM_NAME(UActorSkillTreeComponent, SkillTreeNodes, this);
	// @gdemers cache array before to invoke OnRep on server.
	TArray<USkillTreeNodeObject*> OldItems = SkillTreeNodes;
	for (UObject* StreamableAsset : OutStreamableAssets)
	{
		const auto* SkillTreeNodeObjectClass = Cast<UClass>(StreamableAsset);
		if (!IsValid(SkillTreeNodeObjectClass))
		{
			continue;
		}

		auto* NewSkillTreeNode = NewObject<USkillTreeNodeObject>(this, SkillTreeNodeObjectClass);
		if (!IsValid(NewSkillTreeNode))
		{
			continue;
		}

		// @gdemers initialize our USkillTreeNode runtime representation. This integer encapsulate
		// information about skill type, level, etc...
		int32 PrivateItemId = INDEX_NONE;

		const bool bIsTreeNodeSrcStatic = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::Static);
		if (bIsTreeNodeSrcStatic)
		{
			PrivateItemId = USkillTreeNodeObjectUtils::RuntimeInitStaticItem(Outer,
			                                                                 PrivateSkillTreeNodeIds,
			                                                                 NewSkillTreeNode);
		}
		else
		{
			PrivateItemId = USkillTreeNodeObjectUtils::RuntimeInitOnlineItem(Outer,
			                                                                 PrivateSkillTreeNodeIds,
			                                                                 UActorSkillTreeComponent::GetSkillTreeDataResolverHelper(),
			                                                                 NewSkillTreeNode);
		}

		if (ensureAlwaysMsgf(PrivateItemId != INDEX_NONE, TEXT("Couldn't initialize Tree Node with a valid PrivateId.")) ||
			ensureAlwaysMsgf(!PrivateSkillTreeNodeIds.Contains(PrivateItemId), TEXT("Attempting to initialized a USkillTreeNodeObject with duplicated PrivateItemId value.")))
		{
			PrivateSkillTreeNodeIds.Add(PrivateItemId);
			AddReplicatedSubObject(NewSkillTreeNode);
			SkillTreeNodes.Add(NewSkillTreeNode);
		}
	}

	// @gdemers try to apply gameplay effect on actor based on outer representation.
	TryApplyGameplayEffect(Outer);
}

void UActorSkillTreeComponent::OnRep_SkillTreeNodeCollectionChanged(const TArray<USkillTreeNodeObject*>& OldSkillTreeNodeObjects)
{
}

void UActorSkillTreeComponent::TryApplyGameplayEffect(const AActor* Outer)
{
	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<USkillTreeProvider>(Outer);
	if (!bResult)
	{
		return;
	}

	auto* ResourceManagerComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(Outer);
	if (!ensureAlwaysMsgf(IsValid(ResourceManagerComponent), TEXT("Outer doesn't return valid AVVMResourceManagerComponent!")))
	{
		return;
	}

	USkillTreeNodeObject* NewSkillTreeNodeObject = nullptr;
	if (!SkillTreeNodes.IsEmpty())
	{
		NewSkillTreeNodeObject = SkillTreeNodes.Last();
	}

	if (!IsValid(NewSkillTreeNodeObject))
	{
		return;
	}

	// @gdemers configure runtime state of the item based on if it's held by the outer or just sits in the inventory.
	const bool bIsItemEquipped = IInventoryProvider::Execute_IsItemEquipped(Outer, NewItem);
	if (bIsItemEquipped)
	{
		RequestItemActor(ResourceManagerComponent, NewItem);
	}
}
