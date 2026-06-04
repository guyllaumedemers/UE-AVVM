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
#include "AVVMNotificationSubsystem.h"
#include "AVVMToolkitUtils.h"
#include "SkillSampleModule.h"
#include "SkillTreeExecutionContextParams.h"
#include "SkillTreeExecutionContextRule.h"
#include "SkillTreeNodeObject.h"
#include "SkillTreeNotificationPayload.h"
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
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Tags/PrivateTags.h"

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

	bReplicateUsingRegisteredSubObjectList = false;
}

void UActorSkillTreeComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_AutonomousOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(UActorSkillTreeComponent, SkillTree, Params);
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

	SkillTree.SkillTreeNodeObjects.Reset();
	SkillTreeNodeHandleSystem.Reset();
	PrivateSkillTreeNodeIds.Reset();

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

	SkillTree.SkillTreeNodeObjects.Reset();
	SkillTree.MarkArrayDirty();

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

void UActorSkillTreeComponent::GrantTreeNodeObject(const FSkillTreeNodeObject& NewTreeNodeObject)
{
	const auto Ctx = FExecutionContextParams::Make<FGrantContextParams>(NewTreeNodeObject);
	const auto Rule = GetGrantRule();
	const bool bWasSuccess = CanExecute(Ctx, Rule);
	if (bWasSuccess)
	{
		if ((GetOwnerRole() == ROLE_Authority))
		{
			OnGrant(NewTreeNodeObject);
		}
		else
		{
			Server_GrantTreeNodeObject(NewTreeNodeObject);
		}
	}

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_SKILLSAMPLE_TREENODE_NOTIFICATION_GRANT,
		                                        GetTypedOuter<APlayerController>(),
		                                        GetTypedOuter<AActor>(),
		                                        FAVVMNotificationPayload::Make<FSkillTreeNotificationPayload>(bWasSuccess));
	}
}

void UActorSkillTreeComponent::RevokeTreeNodeObject(const FSkillTreeNodeObject& NewTreeNodeObject)
{
	const auto Ctx = FExecutionContextParams::Make<FRevokeContextParams>(NewTreeNodeObject);
	const auto Rule = GetRevokeRule();
	const bool bWasSuccess = CanExecute(Ctx, Rule);
	if (bWasSuccess)
	{
		if ((GetOwnerRole() == ROLE_Authority))
		{
			OnRevoke(NewTreeNodeObject);
		}
		else
		{
			Server_RevokeTreeNodeObject(NewTreeNodeObject);
		}
	}

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_SKILLSAMPLE_TREENODE_NOTIFICATION_REVOKE,
		                                        GetTypedOuter<APlayerController>(),
		                                        GetTypedOuter<AActor>(),
		                                        FAVVMNotificationPayload::Make<FSkillTreeNotificationPayload>(bWasSuccess));
	}
}

void UActorSkillTreeComponent::ModifyTreeNodeObject(const FSkillTreeModificationContextParams& Params)
{
	const auto Ctx = FExecutionContextParams::Make<FModifyContextParams>(Params.TreeNodeObject, Params.ModifiedLevel);
	const auto Rule = GetModifyRule();
	const bool bWasSuccess = CanExecute(Ctx, Rule);
	if (bWasSuccess)
	{
		if ((GetOwnerRole() == ROLE_Authority))
		{
			OnModify(Params);
		}
		else
		{
			Server_ModifyTreeNodeObject(Params);
		}
	}

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_SKILLSAMPLE_TREENODE_NOTIFICATION_MODIFY,
		                                        GetTypedOuter<APlayerController>(),
		                                        GetTypedOuter<AActor>(),
		                                        FAVVMNotificationPayload::Make<FSkillTreeNotificationPayload>(bWasSuccess));
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

		if (!SkillTreeNodeAsset->CanAccessSkillTreeNodeEffect(NonReplicatedComponentStateTags, NonReplicatedComponentStateTags))
		{
			AVVM_LOGGER_LOG(LogSkillSample,
			                Outer,
			                Outer,
			                TEXT("Failed to meet %s Requirements."),
			                *GetNameSafe(SkillTreeNodeAsset));

			continue;
		}

		DeferredItems.Add(SkillTreeNodeAsset->GetSkillTreeNodeEffectClass().ToSoftObjectPath());
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

TInstancedStruct<FExecutionContextRule> UActorSkillTreeComponent::GetGrantRule() const
{
	return FExecutionContextRule::Make<FGrantRule>();
}

TInstancedStruct<FExecutionContextRule> UActorSkillTreeComponent::GetRevokeRule() const
{
	return FExecutionContextRule::Make<FRevokeRule>();
}

TInstancedStruct<FExecutionContextRule> UActorSkillTreeComponent::GetModifyRule() const
{
	return FExecutionContextRule::Make<FModifyRule>();
}

void UActorSkillTreeComponent::OnGrant(const FSkillTreeNodeObject& NewTreeNodeObject)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	// @gdemers we remove the blocking tag of an already granted GameplayEffect. How is it already granted ?
	// Answer : when opening a menu context, we load them all (based on a Graph Asset definition), and disable
	// what isn't registered with our backend/or disk definition.
	const FGameplayTag BlockingTag = USkillTreeNodeObjectUtils::GetPrivateIdBlockingTag(NewTreeNodeObject.GetSkillTreeNodePrivateId());
	ModifyRuntimeState(NewTreeNodeObject.GetActiveEffectHandleTypeHash(), {}, FGameplayTagContainer{BlockingTag});

	ESkillTreeSrcType OutSrcType = ESkillTreeSrcType::None;
	const bool bIsValid = USkillTreeUtils::GetOuterSourceType(Outer, OutSrcType);
	if (!bIsValid)
	{
		return;
	}

	const bool bIsTreeNodeSrcStatic = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::Static);
	if (bIsTreeNodeSrcStatic)
	{
		CheckDisk();
	}
	else
	{
		CheckBackend();
	}
}

void UActorSkillTreeComponent::OnRevoke(const FSkillTreeNodeObject& NewTreeNodeObject)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	// @gdemers we add the blocking tag of an already granted GameplayEffect. How is it already granted ?
	// Answer : when opening a menu context, we load them all (based on a Graph Asset definition), and disable
	// what isn't registered with our backend/or disk definition.
	const FGameplayTag BlockingTag = USkillTreeNodeObjectUtils::GetPrivateIdBlockingTag(NewTreeNodeObject.GetSkillTreeNodePrivateId());
	ModifyRuntimeState(NewTreeNodeObject.GetActiveEffectHandleTypeHash(), FGameplayTagContainer{BlockingTag}, {});

	ESkillTreeSrcType OutSrcType = ESkillTreeSrcType::None;
	const bool bIsValid = USkillTreeUtils::GetOuterSourceType(Outer, OutSrcType);
	if (!bIsValid)
	{
		return;
	}

	const bool bIsTreeNodeSrcStatic = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::Static);
	if (bIsTreeNodeSrcStatic)
	{
		CheckDisk();
	}
	else
	{
		CheckBackend();
	}
}

void UActorSkillTreeComponent::OnModify(const FSkillTreeModificationContextParams& Params)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	// @gdemers Modify the GameplayEffect level, scaling property values, etc...
	// Level bounds are expected to be handled from within the caller.
	ModifyRuntimeLevel(Params.TreeNodeObject.GetActiveEffectHandleTypeHash(), Params.ModifiedLevel);

	ESkillTreeSrcType OutSrcType = ESkillTreeSrcType::None;
	const bool bIsValid = USkillTreeUtils::GetOuterSourceType(Outer, OutSrcType);
	if (!bIsValid)
	{
		return;
	}

	const bool bIsTreeNodeSrcStatic = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::Static);
	if (bIsTreeNodeSrcStatic)
	{
		CheckDisk();
	}
	else
	{
		CheckBackend();
	}
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

	for (UObject* StreamableAsset : OutStreamableAssets)
	{
		const auto* SkillTreeNodeEffectClass = Cast<UClass>(StreamableAsset);
		if (!IsValid(SkillTreeNodeEffectClass))
		{
			continue;
		}

		const auto* GameplayEffectCDO = SkillTreeNodeEffectClass->GetDefaultObject<UGameplayEffect>();
		if (!IsValid(GameplayEffectCDO))
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
			                                                                 GameplayEffectCDO);
		}
		else
		{
			PrivateItemId = USkillTreeNodeObjectUtils::RuntimeInitOnlineItem(Outer,
			                                                                 PrivateSkillTreeNodeIds,
			                                                                 UActorSkillTreeComponent::GetSkillTreeDataResolverHelper(),
			                                                                 GameplayEffectCDO);
		}

		if (ensureAlwaysMsgf(PrivateItemId != INDEX_NONE, TEXT("Couldn't initialize Tree Node with a valid PrivateId.")) ||
			ensureAlwaysMsgf(!PrivateSkillTreeNodeIds.Contains(PrivateItemId), TEXT("Attempting to initialized a FSkillTreeNodeObject with duplicated PrivateItemId value.")))
		{
			const FActiveGameplayEffectHandle ActiveGameplayEffectHandle = TryApplyGameplayEffect(SkillTreeNodeEffectClass, PrivateItemId);
			const int32 TypeHash = GetTypeHash(ActiveGameplayEffectHandle);
			SkillTree.SkillTreeNodeObjects.Add(FSkillTreeNodeObject(PrivateItemId, TypeHash)/*rvalue*/);
			NonReplicatedActiveGameplayEffectHandles.Add(TypeHash, ActiveGameplayEffectHandle);
			PrivateSkillTreeNodeIds.Add(PrivateItemId);
		}
	}

	SkillTree.MarkArrayDirty();
}

FActiveGameplayEffectHandle UActorSkillTreeComponent::TryApplyGameplayEffect(const UClass* NewGameplayEffectClass,
                                                                             const int32 PrivateTreeNodeId)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return {};
	}

	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(Outer);
	if (!ensureAlwaysMsgf(IsValid(ASC),
	                      TEXT("Missing a valid ASC on the owning outer of the current SkillTreeComponent!")))
	{
		return {};
	}

	TSubclassOf<UGameplayEffect> GameplayEffectClass = const_cast<UClass*>(NewGameplayEffectClass);
	AActor* NonConstOuter = const_cast<AActor*>(Outer);

	// @gdemers filter the level bitmask of our encoded bitmask so we can support progression scaling using GAS.
	const int32 Level = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateTreeNodeId, GET_SKILL_TREE_NODE_LEVEL_ENCODING_BIT_RANGE, GET_SKILL_TREE_NODE_LEVEL_ENCODING_RSHIFT);
	// @gdemers manually grant the GameplayEffect to the ASC, and store the ActiveHandle so we can remove the effect when the owning Outer is no longer referenced
	// within the outer chain of ACharacter, or when a user swap Skill Node entries in UI.
	const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(GameplayEffectClass, NonConstOuter, NonConstOuter, Level);
	const FActiveGameplayEffectHandle ActiveGEHandle = ASC->BP_ApplyGameplayEffectSpecToSelf(GESpecHandle);
	return ActiveGEHandle;
}

bool UActorSkillTreeComponent::CanExecute(const TInstancedStruct<FExecutionContextParams>& Params,
                                          const TInstancedStruct<FExecutionContextRule>& Rule) const
{
	const auto* ContextRule = Rule.GetPtr<FExecutionContextRule>();
	if (!ensureAlwaysMsgf(ContextRule != nullptr, TEXT("FExecutionContextRule invalid.")))
	{
		return false;
	}

	const bool bPredicate = ContextRule->Predicate(this, Params);
	return bPredicate;
}

void UActorSkillTreeComponent::Server_GrantTreeNodeObject_Implementation(const FSkillTreeNodeObject& NewTreeNodeObject)
{
	GrantTreeNodeObject(NewTreeNodeObject);
}

void UActorSkillTreeComponent::Server_RevokeTreeNodeObject_Implementation(const FSkillTreeNodeObject& NewTreeNodeObject)
{
	RevokeTreeNodeObject(NewTreeNodeObject);
}

void UActorSkillTreeComponent::Server_ModifyTreeNodeObject_Implementation(const FSkillTreeModificationContextParams& Params)
{
	ModifyTreeNodeObject(Params);
}

void UActorSkillTreeComponent::ModifyRuntimeState(const int32 SkillTreeNodeTypeHash,
                                                  const FGameplayTagContainer& AddedTags,
                                                  const FGameplayTagContainer& RemovedTags)
{
	const bool bDoesContains = NonReplicatedActiveGameplayEffectHandles.Contains(SkillTreeNodeTypeHash);
	if (!ensureAlwaysMsgf(bDoesContains, TEXT("Attempting to access invalid Type Hash")))
	{
		return;
	}

	auto* ASC = NonReplicatedActiveGameplayEffectHandles[SkillTreeNodeTypeHash].GetOwningAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	if (!AddedTags.IsEmpty())
	{
		ASC->AddLooseGameplayTags(AddedTags);
	}

	if (!RemovedTags.IsEmpty())
	{
		ASC->RemoveLooseGameplayTags(RemovedTags);
	}
}

void UActorSkillTreeComponent::ModifyRuntimeLevel(const int32 SkillTreeNodeTypeHash,
                                                  const int32 NewLevel)
{
	const bool bDoesContains = NonReplicatedActiveGameplayEffectHandles.Contains(SkillTreeNodeTypeHash);
	if (!ensureAlwaysMsgf(bDoesContains, TEXT("Attempting to access invalid Type Hash")))
	{
		return;
	}

	auto& ActiveGameplayEffectHandle = NonReplicatedActiveGameplayEffectHandles[SkillTreeNodeTypeHash];
	auto* ASC = ActiveGameplayEffectHandle.GetOwningAbilitySystemComponent();
	if (IsValid(ASC))
	{
		ASC->SetActiveGameplayEffectLevel(ActiveGameplayEffectHandle, NewLevel);
	}
}

void UActorSkillTreeComponent::CheckBackend() const
{
}

void UActorSkillTreeComponent::CheckDisk() const
{
}
