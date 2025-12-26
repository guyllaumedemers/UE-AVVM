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
#include "AVVMOnlineUtils.h"
#include "AVVMUtils.h"
#include "TriggeringActor.h"
#include "WeaponSample.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"

AActor* FAttachmentSocketTargetingHelper::GetDesiredTypedInner(AActor* Src, AActor* Target) const
{
	if (!IsValid(Src))
	{
		return nullptr;
	}

	const int32 SearchUniqueId = UAVVMGameplayUtils::GetUniqueIdentifier(Src);
	if (!ensureAlwaysMsgf(SearchUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid Class in the Actor Identifier Data Table."),
	                      *Src->GetName()))
	{
		return nullptr;
	}

	TArray<int32> Dependencies;

	auto* Character = Cast<AAVVMCharacter>(Target);
	if (IsValid(Character) && UAVVMUtils::IsNativeScriptInterfaceValid<const IAVVMResourceProvider>(Character))
	{
		// @gdemers fetch {FAVVMPlayerProfile::UniqueId}
		const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Character);
		if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
		                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on content stored in AVVMGameSession."),
		                      *Character->GetName()))
		{
			return nullptr;
		}

		// @gdemers aggregate dependencies defined in backend representation.
		Dependencies = UAVVMOnlineUtils::GetElementDependencies(Character, TargetUniqueId, AAVVMCharacter::GetCharacterDataResolverHelper());

		// @gdemers character dependencies should validate their encoding so the input id we are comparing against isnt an attachment that
		// target a triggering actor.
		Dependencies = UAVVMOnlineEncodingUtils::SearchValues(Dependencies,
		                                                      GET_ITEM_ID_ENCODING_BIT_RANGE,
		                                                      NULL,
		                                                      CHECK_CHARACTER_DEPENDENT_ENCODING);
	}
	else
	{
		auto* TriggeringActor = Cast<ATriggeringActor>(Target);
		if (!IsValid(TriggeringActor))
		{
			return nullptr;
		}

		// @gdemers fetch {FAVVMPlayerResource::UniqueId}
		const int32 TargetUniqueId = UAVVMGameplayUtils::GetUniqueIdentifier(TriggeringActor);
		if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
		                      TEXT("Actor \"%s\" isn't referencing a valid Class in the Actor Identifier Data Table."),
		                      *TriggeringActor->GetName()))
		{
			return nullptr;
		}

		// @gdemers aggregate dependencies defined in backend representation.
		Dependencies = UAVVMOnlineUtils::GetElementDependencies(TriggeringActor->GetTypedOuter<AAVVMCharacter>(),
		                                                        TargetUniqueId,
		                                                        ATriggeringActor::GetTriggeringActorDataResolverHelper());
	}

	// @gdemers from our sub-set of attachments that are equipped to the target actor dependency set, we validate our input attachment against possible match.
	Dependencies = UAVVMOnlineEncodingUtils::SearchValues(Dependencies,
	                                                      GET_ATTACHMENT_ID_ENCODING_BIT_RANGE,
	                                                      GET_ATTACHMENT_ID_ENCODING_RSHIFT,
	                                                      SearchUniqueId);

	if (!Dependencies.IsEmpty())
	{
		return Target;
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

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *AAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName());

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

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *AAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName());

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
	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Parent);
	if (!(SocketDeferral.GetInterface() != nullptr && IsValid(SocketDeferral.GetObject())))
	{
		// @gdemers IMPORTANT : During our first attempt, the received context will reference the AVVMCharacter, but on subsequent attempts, reference
		// the ATriggeringActor which may NOT be the correct target for this attachment (i.e right type, maybe wrong instance). As such, retrieving the interface based on the outer chain
		// is the logic approach to allowing proper recursion until the proper match is found.
		SocketDeferral = Cast<UObject>(Parent->GetImplementingOuterObject(UAVVMDoesSupportSocketDeferral::StaticClass()));
	}

	const bool bDoesImplement = UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	IAVVMDoesSupportSocketDeferral::FOnParentSocketAvailableDelegate::FDelegate Callback;
	Callback.BindUObject(this, &AAttachmentActor::OnSocketParentingDeferred, ContextArgs);
	DeferredSocketParentingDelegateHandle = SocketDeferral->OnSocketParentAvailableDelegate_Add(Callback);
}

void AAttachmentActor::Attach_Implementation(AActor* Target, const FName NewSocketName)
{
	if (!ensureAlwaysMsgf(IsValid(Target), TEXT("Invalid Parent!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Attaching \"%s\" to Outer \"%s\" at SocketName \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Target).GetData(),
	       *AAttachmentActor::StaticClass()->GetName(),
	       *Target->GetName(),
	       *NewSocketName.ToString());

	// @gdemers detach actor + remove AttributeSet registered
	Detach();

	// @gdemers attach actor + add AttributeSet
	AttachToActor(Target, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName);
	OwningOuter = Target;

	// @gdemers attempt registering AttributeSet with ASC. may fail but thats alright! the inventory system handle that case.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->RegisterAttributeSet(OwnedAttributeSet, this);
	}
}

void AAttachmentActor::Detach_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Detaching \"%s\" from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *AAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName());

	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

	// @gdemers clear AttributeSet provided by this attachment.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->UnRegisterAttributeSet(this);
	}
}

void AAttachmentActor::OnSocketParentingDeferred(AActor* Parent,
                                                 AActor* Target,
                                                 const FAVVMSocketTargetingDeferralContextArgs ContextArgs)
{
	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Parent);

	const bool bDoesImplement = UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	// @gdemers we have to update our target from the old context.
	FAVVMSocketTargetingDeferralContextArgs RecursiveContextArgs = ContextArgs;
	RecursiveContextArgs.Parent = Target;

	SocketDeferral->OnSocketParentAvailableDelegate_Remove(DeferredSocketParentingDelegateHandle);
	const bool bIsRooted = FAVVMSocketTargetingHelper::Static_AttachToActorAsync(this, RecursiveContextArgs);
	if (!bIsRooted)
	{
		return;
	}

	// @gdemers Initialized the AttributeSet for the first time based on deferred socketing.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (ensureAlwaysMsgf(IsValid(ASC),
	                     TEXT("New OwningOuter doesn't own a valid ASC.")))
	{
		ASC->SetupAttributeSet(RecursiveContextArgs.SrcAttributeSetSoftObjectPath, Target);
	}
}
