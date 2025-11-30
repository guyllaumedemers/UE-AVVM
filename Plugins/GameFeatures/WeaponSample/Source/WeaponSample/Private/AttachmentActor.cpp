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

#include "AVVMGameplayUtils.h"
#include "AVVMUtils.h"
#include "TriggeringActor.h"
#include "WeaponSample.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"

AActor* FAttachmentSocketTargetingHelper::GetDesiredTypedInner(AActor* Src) const
{
	if (!IsValid(Src))
	{
		return Src;
	}

	TArray<AActor*> Children;
	Src->GetAttachedActors(Children);

	AActor** SearchResult = Children.FindByPredicate([](const AActor* Actor)
	{
		// @gdemers We can further define this later so to narrow the search with more granular constraint.
		const auto* TriggeringActor = Cast<ATriggeringActor>(Actor);
		return IsValid(TriggeringActor);
	});

	return (SearchResult != nullptr) ? *SearchResult : nullptr;
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

	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Parent);

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

	SocketDeferral->OnSocketParentAvailableDelegate_Remove(DeferredSocketParentingDelegateHandle);
	const bool bIsRooted = FAVVMSocketTargetingHelper::Static_AttachToActor(this, ContextArgs);
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
