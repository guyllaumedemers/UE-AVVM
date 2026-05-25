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

#include "AbilitySystemBlueprintLibrary.h"
#include "AVVMLogger.h"
#include "AVVMScopedUtils.h"
#include "AVVMToolkitUtils.h"
#include "SkillSampleModule.h"
#include "SkillTreeNodeObject.h"
#include "SkillTreeProvider.h"
#include "SkillTreeUtils.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineSkillTree.h"
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

	QueueingMechanism = MakeShared<FGameplayEffectSpawnerQueuingMechanism>();

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

	QueueingMechanism.Reset();
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
	                TEXT("Requesting skill tree type %s."),
	                EnumToString(OutSrcType));

	const bool bIsTreeNodeSrcStatic = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::Static);
	if (bIsTreeNodeSrcStatic)
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
	                      TEXT("Attempting to load invalid Skill Tree Node set on Outer \"%s\"."),
	                      *GetNameSafe(Outer)))
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

void UActorSkillTreeComponent::SetupSkillTreeNodeEffects(const TArray<UObject*>& NewResources)
{
	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                      TEXT("Attempting to load invalid Skill Tree Node set on Outer \"%s\"."),
	                      *GetNameSafe(OwningOuter.Get())))
	{
		return;
	}

	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		return;
	}

	USkillTreeNodeObject* SkillTreeNodeObject = QueueingMechanism->PeekItem();
	if (!IsValid(SkillTreeNodeObject) || !ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                                                       TEXT("Attempting to load invalid Ability Definition on SkillTreeNode \"%s\"."),
	                                                       *GetNameSafe(SkillTreeNodeObject)))
	{
		return;
	}

	FOnRequestGameplayEffectClassComplete Callback;
	Callback.BindDynamic(this, &UActorSkillTreeComponent::OnGameplayEffectClassRetrieved);
	SkillTreeNodeObject->GetGameplayEffectClassAsync(NewResources[0], Callback);
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

	if (IsValid(NewSkillTreeNodeObject))
	{
		RequestGameplayEffect(ResourceManagerComponent, NewSkillTreeNodeObject);
	}
}

void UActorSkillTreeComponent::RequestGameplayEffect(UAVVMResourceManagerComponent* ResourceManagerComponent,
                                                     USkillTreeNodeObject* NewSkillTreeNode)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		return;
	}

	const auto RequestGameplayEffectSpawning = [](const TWeakObjectPtr<UActorSkillTreeComponent>& NewActorSkillTreeComponent,
	                                              const TWeakObjectPtr<UAVVMResourceManagerComponent>& NewResourceManagerComponent,
	                                              const TWeakObjectPtr<USkillTreeNodeObject>& SkillTreeNode)
	{
		if (!ensureAlwaysMsgf(NewActorSkillTreeComponent.IsValid(), TEXT("WeakObjectPtr to Actor Skill Tree Component Invalid!")) ||
			!ensureAlwaysMsgf(NewResourceManagerComponent.IsValid(), TEXT("WeakObjectPtr to Resource Manager Component Invalid!")) ||
			!ensureAlwaysMsgf(SkillTreeNode.IsValid(), TEXT("WeakObjectPtr to SkillTreeObject Invalid!")))
		{
			return;
		}

		const AActor* NewOuter = NewActorSkillTreeComponent->OwningOuter.Get();
		AVVM_LOGGER_LOG(LogSkillSample,
		                NewOuter,
		                NewOuter,
		                TEXT("Executing GameplayEffect Request for %s"),
		                *GetNameSafe(SkillTreeNode.Get()));

		NewResourceManagerComponent->RequestAsyncLoading(SkillTreeNode->GetSkillTreeEffectId(), {});
	};

	const auto NewRequest = FOnAsyncSpawnRequestDeferred::CreateWeakLambda(this,
	                                                                       RequestGameplayEffectSpawning,
	                                                                       this,
	                                                                       ResourceManagerComponent,
	                                                                       NewSkillTreeNode);

	const bool bHasPendingRequest = QueueingMechanism->PushDeferredItem(NewSkillTreeNode, NewRequest);
	if (bHasPendingRequest)
	{
		AVVM_LOGGER_LOG(LogSkillSample,
		                Outer,
		                Outer,
		                TEXT("Grant GameplayEffect Request for %s was Deferred."),
		                *GetNameSafe(NewSkillTreeNode));
	}
}

void UActorSkillTreeComponent::OnGameplayEffectClassRetrieved(const UClass* NewGameplayEffectClass,
                                                              USkillTreeNodeObject* NewSkillTreeNode)
{
	const auto ScopedSafety = [](TSharedPtr<FGameplayEffectSpawnerQueuingMechanism> NewQueueingMechanism)
	{
		if (ensureAlwaysMsgf(NewQueueingMechanism.IsValid(),
		                     TEXT("QueueingMechanism invalid!")))
		{
			NewQueueingMechanism->TryExecuteNextRequest(true);
		}
	};

	const auto Callback = FSimpleDelegate::CreateWeakLambda(this, ScopedSafety, QueueingMechanism);
	FAVVMScopedDelegate ScopedDelegate(Callback);

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer),
	                      TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(IsValid(NewSkillTreeNode),
	                      TEXT("Trying to access invalid USkillTreeNodeObject during initialization process.")))
	{
		return;
	}

	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(Outer);
	if (!ensureAlwaysMsgf(IsValid(ASC),
	                      TEXT("Missing a valid ASC on the owning outer of the current SkillTreeComponent!")))
	{
		return;
	}
	
	TSubclassOf<UGameplayEffect> GameplayEffectClass = const_cast<UClass*>(NewGameplayEffectClass);
	AActor* NonConstOuter = const_cast<AActor*>(Outer);

	// @gdemers filter the level bitmask of our encoded bitmask so we can support progression scaling using GAS.
	const int32 PrivateId = USkillTreeNodeObjectUtils::GetPrivateTreeNodeId(NewSkillTreeNode);
	const int32 Level = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateId, GET_SKILL_TREE_NODE_LEVEL_ENCODING_BIT_RANGE, GET_SKILL_TREE_NODE_LEVEL_ENCODING_RSHIFT);
	// @gdemers manually grant the GameplayEffect to the ASC, and store the ActiveHandle so we can remove the effect when the owning Outer is no longer referenced
	// within the outer chain of ACharacter, or when a user swap Skill Node entries in UI.
	const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(GameplayEffectClass, NonConstOuter, NonConstOuter, Level);
	const FActiveGameplayEffectHandle ActiveGEHandle = ASC->BP_ApplyGameplayEffectSpecToSelf(GESpecHandle);
	NewSkillTreeNode->SetActiveGameplayEffectHandle(ActiveGEHandle);
}

UActorSkillTreeComponent::FGameplayEffectSpawnerQueuingMechanism::~FGameplayEffectSpawnerQueuingMechanism()
{
	PendingSpawnRequests.Empty();
	QueuedSkillTreeNodes.Empty();
}

bool UActorSkillTreeComponent::FGameplayEffectSpawnerQueuingMechanism::PushDeferredItem(USkillTreeNodeObject* NewSkillTreeNode,
                                                                                        const UActorSkillTreeComponent::FOnAsyncSpawnRequestDeferred& NewRequest)
{
	QueuedSkillTreeNodes.Add(NewSkillTreeNode);
	PendingSpawnRequests.Add(NewRequest);
	TryExecuteNextRequest();
	return HasPendingRequest();
}

bool UActorSkillTreeComponent::FGameplayEffectSpawnerQueuingMechanism::TryExecuteNextRequest(const bool bCanDequeueFrontItem)
{
	if (bCanDequeueFrontItem && !QueuedSkillTreeNodes.IsEmpty())
	{
		QueuedSkillTreeNodes.RemoveAtSwap(0);
	}

	if (!HasPendingRequest())
	{
		return true;
	}

	const bool bAreContainerEquals = QueuedSkillTreeNodes.Num() == PendingSpawnRequests.Num();
	if (bAreContainerEquals)
	{
		// @gdemers TQueues dont support size...
		FOnAsyncSpawnRequestDeferred NextRequest = PendingSpawnRequests[0];
		PendingSpawnRequests.RemoveAtSwap(0);
		NextRequest.ExecuteIfBound();
	}

	return false;
}

bool UActorSkillTreeComponent::FGameplayEffectSpawnerQueuingMechanism::HasPendingRequest() const
{
	return !PendingSpawnRequests.IsEmpty();
}

USkillTreeNodeObject* UActorSkillTreeComponent::FGameplayEffectSpawnerQueuingMechanism::PeekItem() const
{
	TWeakObjectPtr<USkillTreeNodeObject> SkillTreeNodeObject = nullptr;
	if (!QueuedSkillTreeNodes.IsEmpty())
	{
		SkillTreeNodeObject = QueuedSkillTreeNodes[0].Get();
	}

	return SkillTreeNodeObject.Get();
}
