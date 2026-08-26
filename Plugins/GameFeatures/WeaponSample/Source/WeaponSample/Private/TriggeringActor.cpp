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
#include "Resources/AVVMResourceManagerComponent.h"

TArray<int32> FTriggeringActorDataResolverHelper::GetElementDependencies(const UObject* Outer, const int32 ElementId) const
{
	if (!IsValid(Outer))
	{
		return TArray<int32>{};
	}

	TArray<int32> Dependencies;

	// @gdemers retrieve the character preset, and all items that compose it.
	const auto* Character = Cast<AAVVMCharacter>(Outer);
	if (IsValid(Character) && UAVVMToolkitUtils::IsNativeScriptInterfaceValid<const IAVVMResourceProvider>(Character))
	{
		const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Character);
		Dependencies = UAVVMOnlineBackendUtils::GetElementDependencies(Outer, TargetUniqueId, AAVVMCharacter::GetCharacterDataResolverHelper());
	}

	// @gdemers translate physical addressing into virtual addressing for running searches.
	const int32 VirtualGlobalId = UAVVMOnlineEncodingUtils::EncodeInt32((ElementId/*PhysicalGlobalId*/ - GET_ITEM_PHYSICAL_ADDRESSING_OFFSET),
	                                                                    GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE,
	                                                                    GET_ELEMENT_VIRTUAL_GLOBAL_ID_RSHIFT);

	Dependencies = UAVVMOnlineEncodingUtils::SearchValues(Dependencies,
	                                                      GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE,
	                                                      GET_ELEMENT_VIRTUAL_GLOBAL_ID_RSHIFT,
	                                                      VirtualGlobalId);

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
		IAVVMDoesActorSupportDeferredSocketParenting::Execute_Detach(this);
	}
#endif
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
	SparseClassData->TriggeringAbilityClass = TriggeringAbilityClass_DEPRECATED;
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

void ATriggeringActor::SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet)
{
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
	if (!IsActorBeginningPlay())
	{
		FinishSpawning(GetTransform());
	}

	// @gdemers detach actor from root
	IAVVMDoesActorSupportDeferredSocketParenting::Execute_Detach(this);

	// @gdemers attach actor to root, and update OwningOuter
	ensureAlwaysMsgf(AttachToActor(Target, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName), TEXT("Failed to find socket target."));
	OwningOuter = Target;

	// @gdemers bind animation, and attribute set with new owning outer
	IAVVMDoesActorSupportStateBinding::Execute_Bind(this);
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

	// @gdemers Unregister/Register ability from owner.
	Server_SwapAbility(true);

	// @gdemers attempt registering AttributeSet with ASC. may fail but thats alright! the inventory system handle that case.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->RegisterAttributeSet(OwnedAttributeSet, this);
	}

	// @gdemers allow linking anim instance to driving anim instance.
	auto* TargetSkeletalMeshComponent = Outer->GetComponentByClass<USkeletalMeshComponent>();
	if (IsValid(TargetSkeletalMeshComponent))
	{
		TargetSkeletalMeshComponent->LinkAnimClassLayers(GetLinkedAnimInstanceClass());
	}
	
	// @gdemers notify loadout system to attempt default equipping ourself if we are targeting the correct slot.
	NotifyOnNewActorStateBound();
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
	                this,
	                TEXT("Unbind Target."));

	// @gdemers Unregister ability from owner.
	Server_SwapAbility(false);

	// @gdemers clear AttributeSet provided by this attachment.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->UnRegisterAttributeSet(this);
	}

	// @gdemers allow unlinking anim instance from driving anim instance.
	auto* TargetSkeletalMeshComponent = Outer->GetComponentByClass<USkeletalMeshComponent>();
	if (IsValid(TargetSkeletalMeshComponent))
	{
		TargetSkeletalMeshComponent->UnlinkAnimClassLayers(GetLinkedAnimInstanceClass());
	}
}

void ATriggeringActor::Restart_Implementation()
{
	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					this,
					TEXT("Restart animation"));
}

void ATriggeringActor::Pause_Implementation()
{
	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					this,
					TEXT("Pause animation"));
}

void ATriggeringActor::Resume_Implementation()
{
	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					this,
					TEXT("Resume animation"));
}

void ATriggeringActor::Flush_Implementation()
{
	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					this,
					TEXT("Flush animation"));
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
	
	// TODO @gdemers we have find a proper root, and can initialize. We however may want to only grant an attribute set
	// if the element is active, and not equipped which are two unique states.

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
	// @gdemers IMPORTANT : we are not passing through the AVVMResourceManagerComponent here to async load the GameplayAbility class.
	// Doing so would prevent caching of the Ability and removal of it during context switching of triggering actors. (i.e during weapon switch, etc...)
	FStreamableDelegate OnRequestTriggeringActorAbilityComplete;
	OnRequestTriggeringActorAbilityComplete.BindUObject(this, &ATriggeringActor::OnTriggeringAbilityClassAcquired);
	TriggeringAbilityClassHandle = UAssetManager::Get().LoadAssetList({GetTriggeringAbilityClass().ToSoftObjectPath()});
}

void ATriggeringActor::UnRegisterAbility()
{
	if (!TriggeringAbilitySpecHandle.IsValid())
	{
		return;
	}

	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
	if (IsValid(ASC))
	{
		ASC->ClearAbility(TriggeringAbilitySpecHandle);
	}
}

void ATriggeringActor::OnTriggeringAbilityClassAcquired()
{
	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
	if (!TriggeringAbilityClassHandle.IsValid() || !ensureAlwaysMsgf(IsValid(ASC),
	                                                                 TEXT("Owning Outer missing valid ASC.")))
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	TriggeringAbilityClassHandle->GetLoadedAssets(OutStreamableAssets);

	if (OutStreamableAssets.IsEmpty())
	{
		return;
	}

	auto* GameplayAbilityClass = Cast<UClass>(OutStreamableAssets[0]);
	if (!IsValid(GameplayAbilityClass))
	{
		return;
	}

	TriggeringAbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec
	                                               {
			                                               GameplayAbilityClass,
			                                               1,
			                                               GameplayAbilityClass->GetDefaultObject<UAVVMGameplayAbility>()->GetInputId()
	                                               });
}

const FDataRegistryId ATriggeringActor::GetConditionalTriggeringDefinition() const
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
