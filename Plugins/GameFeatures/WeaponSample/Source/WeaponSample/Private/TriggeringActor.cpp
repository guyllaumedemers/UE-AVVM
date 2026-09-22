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
#include "TriggeringActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AVVMCharacter.h"
#include "AVVMGameplayUtils.h"
#include "AVVMLogger.h"
#include "AVVMReplicatedTagComponent.h"
#include "AVVMToolkitUtils.h"
#include "WeaponSampleModule.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Ability/AVVMGameplayAbility.h"
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Tags/PrivateTags.h"

TArray<int32> FTriggeringActorDataResolverHelper::GetElementDependencies(const UObject* Outer, const int32 ElementId) const
{
	if (!IsValid(Outer))
	{
		return TArray<int32>{};
	}

	const int32 TargetInstancedId = IAVVMDoesActorSupportInstanceIdentifier::Execute_GetInstancedId(Outer);
	if (!ensureAlwaysMsgf(TargetInstancedId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid instanced id."),
	                      *GetNameSafe(Outer)))
	{
		return TArray<int32>{};
	}

	TArray<int32> Dependencies{};

	const auto* Character = Cast<AAVVMCharacter>(Outer->GetTypedOuter<AAVVMCharacter>());
	if (IsValid(Character) && UAVVMToolkitUtils::IsNativeScriptInterfaceValid<const IAVVMResourceProvider>(Character))
	{
		// @gdemers retrieve the inventory dependency graph, and lookup for our attachment.
		const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Character);
		Dependencies = UAVVMOnlineBackendUtils::GetElementDependencies(Character, TargetUniqueId, AAVVMCharacter::GetCharacterDataResolverHelper());
	}

	Dependencies.RemoveAll([SearchVirtualGlobalId = (ElementId/*PhysicalGlobalId*/ - GET_ITEM_PHYSICAL_ADDRESSING_OFFSET), SearchInstancedId = TargetInstancedId](const int32 NewPrivateItemId)
	{
		const int32 OtherVirtualGlobalId = UAVVMOnlineEncodingUtils::DecodeInt32(NewPrivateItemId, GET_ITEM_LOOKUP_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_ITEM_LOOKUP_VIRTUAL_GLOBAL_ID_RSHIFT);
		const int32 OtherInstancedId = UAVVMOnlineEncodingUtils::DecodeInt32(NewPrivateItemId, GET_ITEM_LOOKUP_INSTANCED_ID_BIT_RANGE, GET_ITEM_LOOKUP_INSTANCED_ID_RSHIFT);
		return (true == !!(SearchVirtualGlobalId ^ OtherVirtualGlobalId)) || (true == !!(SearchInstancedId ^ OtherInstancedId)/*XOR 0 on equality, 1 on inequality*/);
	});

	return Dependencies;
}

AActor* FTriggeringSocketTargetingHelper::GetDesiredTypedInner(AActor* Src, AActor* Target) const
{
	if (!IsValid(Target) || !Target->IsA<ACharacter>())
	{
		return nullptr;
	}
	else
	{
		return Target;
	}
}

ATriggeringActor::ATriggeringActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAbilitySystemComponent>(this, TEXT("MAbilitySystemComponent"));
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));
	ReplicatedTagComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMReplicatedTagComponent>(this, TEXT("ReplicatedTagComponent"));

	// @gdemers if tick is required, start an AbilityTask_Tick, and kill the process on completion.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickBatching = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	SetReplicateMovement(true);
	bReplicates = true;
}

void ATriggeringActor::BeginPlay()
{
	Super::BeginPlay();

	auto* Outer = GetOwner();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	OwningOuter = Outer;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(ATriggeringActor::StaticClass()));

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		// @gdemers Resource loading process has race conditions due to requiring Outer actor being cached.
		// This early out the process, and require deferral/secondary invocation.
		BP_PostOuterSet();
		
		if (IsValid(ReplicatedTagComponent))
		{
			// @gdemers better use this custom Tag component than the ASC as handling
			// Allowed Tags is simpler, than Blocked Tags which is only what the ASC has exposed.
			ReplicatedTagComponent->ModifyFilteredTags(GetSupportedTriggeringModes(), {});
		}

		auto SocketDeferral = TScriptInterface<IAVVMSocketProcessHandler>(Outer);
		if (ensureAlwaysMsgf(UAVVMToolkitUtils::IsNativeScriptInterfaceValid(SocketDeferral),
		                     TEXT("Outer doesn't implement required interface.")))
		{
			SocketDeferral->NotifyOnNewSocketParentAvailable(this);
		}

		if (GetTriggeringActorSparseData(EGetSparseClassDataMethod::ArchetypeIfNull)->bShouldSwapAbilityOnBeginPlay)
		{
			Server_SwapAbility(true);
		}
	}
#endif
}

void ATriggeringActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(ATriggeringActor::StaticClass()));

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		// @gdemers unbind abilities from owning outer.
		UnRegisterAbility();

		IAVVMDoesActorSupportDeferredSocketParenting::Execute_Detach(this);
		if (IsValid(ReplicatedTagComponent))
		{
			ReplicatedTagComponent->ModifyFilteredTags({}, GetSupportedTriggeringModes());
		}
	}
#endif
}

void ATriggeringActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	// @gdemers the only thing required for replication here!
	DOREPLIFETIME_WITH_PARAMS_FAST(ATriggeringActor, OwnedAttributeSet, Params);
}

#if WITH_EDITOR
void ATriggeringActor::MoveDataToSparseClassDataStruct() const
{
	// make sure we don't overwrite the sparse data if it has been saved already
	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass == nullptr || BPClass->bIsSparseClassDataSerializable == true)
	{
		return;
	}

	Super::MoveDataToSparseClassDataStruct();

#if WITH_EDITORONLY_DATA
	// Unreal Header Tool (UHT) will create GetMySparseClassData automatically.
	FTriggeringActorSparseData* SparseClassData = GetMutableTriggeringActorSparseData();

	// Modify these lines to include all Sparse Class Data properties.
	SparseClassData->LinkedAnimInstanceClass = LinkedAnimInstanceClass_DEPRECATED;
	SparseClassData->bShouldSwapAbilityOnBeginPlay = bShouldSwapAbilityOnBeginPlay_DEPRECATED;
	SparseClassData->TriggeringDefinitionId = TriggeringDefinitionId_DEPRECATED;
#endif // WITH_EDITORONLY_DATA
}
#endif

void ATriggeringActor::Server_SwapAbility_Implementation(const bool bIsActive)
{
	UnRegisterAbility();

	if (bIsActive)
	{
		RegisterAbility();
	}
}

UAbilitySystemComponent* ATriggeringActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

const UAttributeSet* ATriggeringActor::GetAttributeSet_Implementation() const
{
	return OwnedAttributeSet;
}

void ATriggeringActor::SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ATriggeringActor, OwnedAttributeSet, this);
	OwnedAttributeSet = NewAttributeSet;
}

TInstancedStruct<FAVVMSocketTargetingHelper> ATriggeringActor::GetSocketHelper_Implementation() const
{
	return FAVVMSocketTargetingHelper::Make<FTriggeringSocketTargetingHelper>();
}

void ATriggeringActor::DeferredSocketParenting_Implementation(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	AActor* Parent = ContextArgs.Parent.Get();
	if (!IsValid(Parent))
	{
		return;
	}

	auto SocketDeferral = TScriptInterface<IAVVMSocketProcessHandler>(Parent);

	const bool bDoesImplement = UAVVMToolkitUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	IAVVMSocketProcessHandler::FOnNewSocketParentAvailableDelegate::FDelegate Callback;
	Callback.BindUObject(this, &ATriggeringActor::OnSocketParentingDeferred, ContextArgs);
	DeferredSocketParentingDelegateHandle = SocketDeferral->OnNewSocketParentAvailableDelegate_Add(Callback);
}

void ATriggeringActor::Attach_Implementation(AActor* Target, const FGameplayTag& NewItemAttachmentSlotTag, const FName NewSocketName)
{
	if (!ensureAlwaysMsgf(IsValid(Target), TEXT("Invalid Parent!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Target,
	                TEXT("Root Parent used to attach at socket name %s."),
	                *NewSocketName.ToString());
	
	// @gdemers Actor creation should invoke begin play ONLY upon attaching,
	// and not on creation.
	Rename(nullptr, Target);
	if (!IsActorInitialized())
	{
		FinishSpawning(GetTransform());
	}

	// @gdemers detach actor from root
	IAVVMDoesActorSupportDeferredSocketParenting::Execute_Detach(this);

	// @gdemers attach actor to root, and update OwningOuter
	ensureAlwaysMsgf(AttachToActor(Target, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName), TEXT("Failed to find socket target."));
	OwningOuter = Target;

	// EDIT - We shouldnt be binding here. Wait for the loadout system to handle equipping an entry, and binding!
	// @gdemers bind animation, and attribute set with new owning outer
	// IAVVMDoesActorSupportStateBinding::Execute_Bind(this);
	
	// @gdemers notify loadout system to attempt default equipping ourself if we are targeting the correct slot.
	NotifyOnNewActorStateBound();
}

void ATriggeringActor::Detach_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Detach from Root Parent."));

	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	IAVVMDoesActorSupportStateBinding::Execute_Unbind(this);
}

TSubclassOf<UAnimMontage> ATriggeringActor::GetMontageClassByTag_Implementation(const FGameplayTag& MontageTag) const
{
	const auto& TriggeringActorMontages = GetMontages();
	if (ensureAlwaysMsgf(TriggeringActorMontages.Contains(MontageTag),
	                     TEXT("Invalid tag in the montage list.")))
	{
		return TriggeringActorMontages[MontageTag];
	}
	else
	{
		return nullptr;
	}
}

void ATriggeringActor::Bind_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Bind to Target."));

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		// @gdemers Unregister/Register ability from owner.
		Server_SwapAbility(true);

		// @gdemers attempt registering AttributeSet with ASC. may fail but thats alright! the inventory system handle that case.
		auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
		if (IsValid(ASC))
		{
			ASC->RegisterAttributeSet(OwnedAttributeSet, this);
		}
	}
#endif

	// @gdemers allow linking anim instance to driving anim instance.
	auto* TargetSkeletalMeshComponent = Outer->GetComponentByClass<USkeletalMeshComponent>();
	if (IsValid(TargetSkeletalMeshComponent))
	{
		TargetSkeletalMeshComponent->LinkAnimClassLayers(GetLinkedAnimInstanceClass());
	}
}

void ATriggeringActor::Unbind_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Unbind Target."));

#if WITH_SERVER_CODE
	// @gdemers predictive weapon selection will execute both server-client.
	if (HasAuthority())
	{
		// @gdemers Unregister ability from owner.
		Server_SwapAbility(false);

		// @gdemers clear AttributeSet provided by this attachment.
		auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
		if (IsValid(ASC))
		{
			ASC->UnRegisterAttributeSet(this);
		}
	}
#endif

	// @gdemers allow unlinking anim instance from driving anim instance.
	auto* TargetSkeletalMeshComponent = Outer->GetComponentByClass<USkeletalMeshComponent>();
	if (IsValid(TargetSkeletalMeshComponent))
	{
		TargetSkeletalMeshComponent->UnlinkAnimClassLayers(GetLinkedAnimInstanceClass());
	}
}

void ATriggeringActor::ApplyComplexVisibilityToSelf_Implementation()
{
	// @gdemers impl in BP visibility change requirements.
	BP_SetComplexVisibilityToSelf(IsHidden());
}

void ATriggeringActor::Restart_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}
	
	IAVVMDoesActorSupportStateBinding::Execute_Bind(this);
	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					Outer,
					TEXT("Restart animation"));
	
	FGameplayEventData GAS_EventData{};
	GAS_EventData.Instigator = this;
	GAS_EventData.Target = Outer;
	// @gdemers notify locally predicted GAS "ToggleEquip" Ability to play "equip" montage.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(const_cast<AActor*>(Outer), TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_DRAW, MoveTemp(GAS_EventData));
}

void ATriggeringActor::Pause_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}
	
	// @gdemers We will not be support an unequip animation as mentioned in the EquipAbility class description,
	// which is why we allow Unbinding the linked animation on the active weapon before transitioning with the next target weapon.
	// example ref : Warhammer 40k - Darktide.
	IAVVMDoesActorSupportStateBinding::Execute_Unbind(this);
	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Pause animation"));
	
	FGameplayEventData GAS_EventData{};
	GAS_EventData.Instigator = this;
	GAS_EventData.Target = Outer;
	// @gdemers IMPORTANT possible truncation here.
	GAS_EventData.EventMagnitude = UAVVMToolkitUtils::GetServerWorldTime(this); // timestamp to cache on the ability to pause/resume montage at correct time during pause/resume segment.
	// @gdemers notify locally predicted GAS "ToggleEquip" Ability to interrupt/cancel "equip" montage. a secondary event will trigger to play the equipping of another instance.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(const_cast<AActor*>(Outer), TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_HOLSTERED, MoveTemp(GAS_EventData));
}

void ATriggeringActor::Resume_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}
	
	IAVVMDoesActorSupportStateBinding::Execute_Bind(this);
	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                this,
	                TEXT("Resume animation"));
	
	FGameplayEventData GAS_EventData{};
	GAS_EventData.Instigator = this;
	GAS_EventData.Target = Outer;
	// @gdemers IMPORTANT possible truncation here.
	GAS_EventData.EventMagnitude = UAVVMToolkitUtils::GetServerWorldTime(this); // timestamp to cache on the ability to pause/resume montage at correct time during pause/resume segment.
	// @gdemers notify locally predicted GAS "ToggleEquip" Ability to play "equip" montage.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(const_cast<AActor*>(Outer), TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_DRAW, MoveTemp(GAS_EventData));
}

void ATriggeringActor::Flush_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}
	
	// @gdemers We will not be support an unequip animation as mentioned in the EquipAbility class description,
	// which is why we allow Unbinding the linked animation on the active weapon before transitioning with the next target weapon.
	// example ref : Warhammer 40k - Darktide.
	IAVVMDoesActorSupportStateBinding::Execute_Unbind(this);
	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					Outer,
					TEXT("Flush animation"));
	
	// TODO @gdemers determine what are the requirements for flushing the pipeline
}

int32 ATriggeringActor::GetProviderUniqueId_Implementation() const
{
	return UAVVMGameplayUtils::GetActorUniqueIdentifierByActor(this);
}

UAVVMResourceManagerComponent* ATriggeringActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> ATriggeringActor::GetResourceDefinitionRegistryIds_Implementation() const
{
	return {GetConditionalTriggeringDefinition()};
}

const TInstancedStruct<FAVVMDataResolverHelper>& ATriggeringActor::GetTriggeringActorDataResolverHelper()
{
	static auto Helper = FAVVMDataResolverHelper::Make<FTriggeringActorDataResolverHelper>();
	return Helper;
}

void ATriggeringActor::OnSocketParentingDeferred(AActor* Parent,
                                                 AActor* Target,
                                                 const FAVVMSocketTargetingDeferralContextArgs ContextArgs)
{
	auto SocketDeferral = TScriptInterface<IAVVMSocketProcessHandler>(Parent);

	const bool bDoesImplement = UAVVMToolkitUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	SocketDeferral->OnNewSocketParentAvailableDelegate_Remove(DeferredSocketParentingDelegateHandle);
	const bool bIsRooted = FAVVMSocketTargetingHelper::Static_AttachToActorAsync(this, ContextArgs);
	if (!bIsRooted)
	{
		return;
	}

	// @gdemers Initialized the AttributeSet for the first time based on deferred socketing.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (ensureAlwaysMsgf(IsValid(ASC),
	                     TEXT("New OwningOuter doesn't own a valid ASC.")))
	{
		ASC->SetupAttributeSet(ContextArgs.SrcAttributeSetSoftObjectPath, Target);
	}
}

void ATriggeringActor::RegisterAbility()
{
	if (StreamableHandle.IsValid())
	{
		return;
	}

	TArray<FSoftObjectPath> ResourcePaths{};
	for (const auto& AbilityClass : GetAbilityClasses())
	{
		ResourcePaths.Add(AbilityClass.ToSoftObjectPath());
	}

	FStreamableDelegate Callback{};
	Callback.BindUObject(this, &ATriggeringActor::OnTriggeringAbilityClassAcquired);
	StreamableHandle = UAssetManager::Get().LoadAssetList(ResourcePaths, Callback);
}

void ATriggeringActor::UnRegisterAbility()
{
	if (TriggeringAbilitySpecHandles.IsEmpty())
	{
		return;
	}

	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
	if (!IsValid(ASC))
	{
		return;
	}

	for (const auto& Handle : TriggeringAbilitySpecHandles)
	{
		ASC->ClearAbility(Handle);
	}

	TriggeringAbilitySpecHandles.Reset();
	StreamableHandle.Reset();
}

void ATriggeringActor::OnTriggeringAbilityClassAcquired()
{
	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
	if (!StreamableHandle.IsValid() || !ensureAlwaysMsgf(IsValid(ASC),
	                                                                 TEXT("Owning Outer missing valid ASC.")))
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	StreamableHandle->GetLoadedAssets(OutStreamableAssets);

	for (auto* OutStreamableAsset : OutStreamableAssets)
	{
		auto* GameplayAbilityClass = Cast<UClass>(OutStreamableAsset);
		if (!IsValid(GameplayAbilityClass))
		{
			return;
		}

		const auto AbilitySpec = FGameplayAbilitySpec
		{
				GameplayAbilityClass,
				1,
				GameplayAbilityClass->GetDefaultObject<UAVVMGameplayAbility>()->GetInputId(),
				this /*provide us as source object so we can differentiate when executing the ability the source of execution*/
		};

		TriggeringAbilitySpecHandles.Add(ASC->GiveAbility(AbilitySpec));
	}
}

FDataRegistryId ATriggeringActor::GetConditionalTriggeringDefinition() const
{
	const bool bResult = GetTriggeringActorSparseData(EGetSparseClassDataMethod::ArchetypeIfNull)->bDoesDefineAttachmentStatically;
	return bResult ? GetTriggeringDefinitionId() : FDataRegistryId{};
}

void UTriggeringUtils::Swap(AActor* UnEquip,
                            AActor* Equip,
                            const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	FAVVMSocketTargetingHelper::Static_Detach(UnEquip);
	// @gdemers this api is used during deterministic case, where the attachment is created based on the TriggeringActor
	// Definition. i.e Our Actor creation request comes from instancing sub-registry withing the Outer Actor Definition.
	// As such, async request to find the correct parent isnt required. It's already owned, most-likely called from within
	// the parent actor.
	FAVVMSocketTargetingHelper::Static_AttachToActor(Equip, ContextArgs);
}
