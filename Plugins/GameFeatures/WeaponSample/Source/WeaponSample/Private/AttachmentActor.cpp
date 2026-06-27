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
#include "AttachmentActor.h"

#include "AVVMCharacter.h"
#include "AVVMGameplayUtils.h"
#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
#include "TriggeringActor.h"
#include "WeaponSampleModule.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Components/SkeletalMeshComponent.h"

AActor* FAttachmentSocketTargetingHelper::GetDesiredTypedInner(AActor* Src, AActor* Target) const
{
	if (!IsValid(Src) || !UAVVMToolkitUtils::IsNativeScriptInterfaceValid<const IAVVMResourceProvider>(Target))
	{
		return nullptr;
	}

	// @gdemers src (attachment) unique id defined in global table. Note : attachment shouldn't be considered
	// as resource provider. They are instanced from a resource provider which is different.
	const int32 PhysicalGlobalId = UAVVMGameplayUtils::GetActorUniqueIdentifierByActor(Src);
	if (!ensureAlwaysMsgf(PhysicalGlobalId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid Class in the Actor Identifier Data Table."),
	                      *Src->GetName()))
	{
		return nullptr;
	}

	// @gdemers target (character {FAVVMPlayerProfile::UniqueId}/or triggering actor) unique id.
	const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Target);
	if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on IAVVMResourceProvider::GetProviderUniqueId implementation."),
	                      *Target->GetName()))
	{
		return nullptr;
	}

	TArray<int32> Dependencies;

	const auto* Character = Cast<AAVVMCharacter>(Target);
	if (IsValid(Character))
	{
		// @gdemers aggregate dependencies defined in backend representation.
		Dependencies = UAVVMOnlineBackendUtils::GetElementDependencies(Character, TargetUniqueId, AAVVMCharacter::GetCharacterDataResolverHelper());

		// @gdemers character dependencies should validate their encoding so the input id we are comparing against
		// isnt an attachment that target a triggering actor.
		Dependencies = UAVVMOnlineEncodingUtils::SearchValues(Dependencies,
		                                                      GET_ELEMENT_RELATIONSHIP_BIT_RANGE,
		                                                      GET_ELEMENT_RELATIONSHIP_RSHIFT,
		                                                      FILTER_CHARACTER_RELATIONSHIP_BIT);
	}
	else
	{
		const auto* TriggeringActor = Cast<ATriggeringActor>(Target);
		if (!IsValid(TriggeringActor))
		{
			return nullptr;
		}

		// @gdemers aggregate dependencies defined in backend representation.
		Dependencies = UAVVMOnlineBackendUtils::GetElementDependencies(TriggeringActor->GetTypedOuter<AAVVMCharacter>(),
		                                                               TargetUniqueId/*ProviderId of target is the PhysicalGlobalId*/,
		                                                               ATriggeringActor::GetTriggeringActorDataResolverHelper());
	}

	// @gdemers translate physical addressing into virtual addressing for running searches.
	const int32 VirtualGlobalId = UAVVMOnlineEncodingUtils::EncodeInt32((PhysicalGlobalId - GET_ATTACHMENT_PHYSICAL_ADDRESSING_OFFSET),
	                                                                    GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE,
	                                                                    GET_ELEMENT_VIRTUAL_GLOBAL_ID_RSHIFT);

	// @gdemers search for virtual address within bit range
	Dependencies = UAVVMOnlineEncodingUtils::SearchValues(Dependencies,
	                                                      GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE,
	                                                      GET_ELEMENT_VIRTUAL_GLOBAL_ID_RSHIFT,
	                                                      VirtualGlobalId);

	if (!Dependencies.IsEmpty())
	{
		return Target;
	}

	// @gdemers early out for the recursive case so we avoid calling GetAllChildActors.
	if (Target->IsA<ATriggeringActor>())
	{
		return nullptr;
	}

	// @gdemers we need to validate that the parent we are looking for hasnt been created yet.
	TArray<AActor*> OutChildren;
	Target->GetAllChildActors(OutChildren, false);

	OutChildren = OutChildren.FilterByPredicate([](const AActor* Child)
	{
		return IsValid(Child) && Child->IsA<ATriggeringActor>();
	});

	for (AActor* Child : OutChildren)
	{
		// @gdemers recursively search for parent that are already spawned, and may own the current attachment.
		AActor* SearchResult = FAttachmentSocketTargetingHelper::GetDesiredTypedInner(Src, Child);
		if (IsValid(SearchResult))
		{
			return SearchResult;
		}
	}

	return nullptr;
}

AAttachmentActor::AAttachmentActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// @gdemers if tick is required, start an AbilityTask_Tick, and kill the process on completion.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickBatching = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	SetReplicateMovement(true);
	bReplicates = true;
}

void AAttachmentActor::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(AAttachmentActor::StaticClass()));

	OwningOuter = Outer;
}

void AAttachmentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(AAttachmentActor::StaticClass()));

	Detach();
}

UAbilitySystemComponent* AAttachmentActor::GetAbilitySystemComponent() const
{
	return UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
}

void AAttachmentActor::SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet)
{
	OwnedAttributeSet = NewAttributeSet;
}

TInstancedStruct<FAVVMSocketTargetingHelper> AAttachmentActor::GetSocketHelper_Implementation() const
{
	return FAVVMSocketTargetingHelper::Make<FAttachmentSocketTargetingHelper>();
}

void AAttachmentActor::DeferredSocketParenting_Implementation(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	AActor* Parent = ContextArgs.Parent.Get();
	if (!IsValid(Parent))
	{
		return;
	}

	// @gdemers retrieve socket deferral interface from current parent.
	auto SocketDeferral = TScriptInterface<IAVVMSocketProcessHandler>(Parent);
	if (!(SocketDeferral.GetInterface() != nullptr && IsValid(SocketDeferral.GetObject())))
	{
		// @gdemers IMPORTANT : During our first traversal, the received context will reference the AVVMCharacter, but on deferred attempts,
		// the parent type may reference a ATriggeringActor which will not implement the interface UAVVMSocketProcessHandler.
		// IMPORTANT : ONLY the AVVMCharacter should orchestrate the socketing process.
		SocketDeferral = Cast<UObject>(Parent->GetImplementingOuterObject(UAVVMSocketProcessHandler::StaticClass()));
	}

	const bool bDoesImplement = UAVVMToolkitUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	IAVVMSocketProcessHandler::FOnNewSocketParentAvailableDelegate::FDelegate Callback;
	Callback.BindUObject(this, &AAttachmentActor::OnSocketParentingDeferred, ContextArgs);
	DeferredSocketParentingDelegateHandle = SocketDeferral->OnNewSocketParentAvailableDelegate_Add(Callback);
}

void AAttachmentActor::Attach_Implementation(AActor* Target, const FGameplayTag& NewItemAttachmentSlotTag, const FName NewSocketName)
{
	if (!ensureAlwaysMsgf(IsValid(Target), TEXT("Invalid Parent!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Target,
	                TEXT("Target for attach at socket name %s."),
	                *NewSocketName.ToString());

	// @gdemers detach actor + remove AttributeSet registered
	IAVVMDoesActorSupportDeferredSocketParenting::Execute_Detach(this);

	// @gdemers attach actor + add AttributeSet
	AttachToActor(Target, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName);
	OwningOuter = Target;

	const bool bShouldNotifyWhenAttachingActor = Target->Implements<UAVVMDoesActorSupportOnAttachmentNotify>();
	if (bShouldNotifyWhenAttachingActor)
	{
		OwningSocketSlotTag = NewItemAttachmentSlotTag;

		const auto Observer = TScriptInterface<const IAVVMDoesActorSupportOnAttachmentNotify>(Target);
		Observer->NotifyOnNewSocketAttached(NewItemAttachmentSlotTag, this);
	}
	
	// TODO @gdemers we have find a proper root, and can initialize. We however may want to only grant an attribute set
	// if the element is active, and not equipped which are two unique states.

	// @gdemers attempt registering AttributeSet with ASC. may fail but thats alright! the inventory system handle that case.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->RegisterAttributeSet(OwnedAttributeSet, this);
	}
	
	// @gdemers allow linking anim instance to driving anim instance.
	auto* TargetSkeletalMeshComponent = Target->GetComponentByClass<USkeletalMeshComponent>();
	if (IsValid(TargetSkeletalMeshComponent))
	{
		TargetSkeletalMeshComponent->LinkAnimClassLayers(LinkedAnimInstanceClass);
	}
}

void AAttachmentActor::Detach_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Target for detach."));

	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

	const bool bShouldNotifyWhenDetachingActor = Outer->Implements<UAVVMDoesActorSupportOnAttachmentNotify>();
	if (bShouldNotifyWhenDetachingActor)
	{
		const auto Observer = TScriptInterface<const IAVVMDoesActorSupportOnAttachmentNotify>(Outer);
		Observer->NotifyOnNewSocketDetached(OwningSocketSlotTag);
	}

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
		TargetSkeletalMeshComponent->UnlinkAnimClassLayers(LinkedAnimInstanceClass);
	}
}

void AAttachmentActor::OnSocketParentingDeferred(AActor* Parent,
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

	// @gdemers we have to update our target from the old context.
	FAVVMSocketTargetingDeferralContextArgs RecursiveContextArgs = ContextArgs;
	RecursiveContextArgs.Parent = Target;

	SocketDeferral->OnNewSocketParentAvailableDelegate_Remove(DeferredSocketParentingDelegateHandle);
	const bool bIsRooted = FAVVMSocketTargetingHelper::Static_AttachToActorAsync(this, RecursiveContextArgs);
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
		ASC->SetupAttributeSet(RecursiveContextArgs.SrcAttributeSetSoftObjectPath, Target);
	}
}
