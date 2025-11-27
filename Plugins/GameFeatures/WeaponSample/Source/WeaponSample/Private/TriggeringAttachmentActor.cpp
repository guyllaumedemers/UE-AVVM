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
#include "TriggeringAttachmentActor.h"

#include "AVVMGameplayUtils.h"
#include "AVVMUtils.h"
#include "TriggeringActor.h"
#include "WeaponSample.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"

AActor* FTriggeringAttachmentSocketTargetingHelper::GetDesiredTypedInner(AActor* Src) const
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

	return (SearchResult != nullptr) ? *SearchResult : Src;
}

ATriggeringAttachmentActor::ATriggeringAttachmentActor(const FObjectInitializer& ObjectInitializer)
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

void ATriggeringAttachmentActor::BeginPlay()
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
	       *ATriggeringAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter = Outer;
}

void ATriggeringAttachmentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
	       *ATriggeringAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName());

	Detach();
}

void ATriggeringAttachmentActor::Attach(AActor* Parent)
{
	if (!ensureAlwaysMsgf(IsValid(Parent), TEXT("Invalid Parent!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Attaching \"%s\" to Outer \"%s\" at SocketName \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Parent).GetData(),
	       *ATriggeringAttachmentActor::StaticClass()->GetName(),
	       *Parent->GetName(),
	       *SocketName.ToString());

	// @gdemers detach actor + remove AttributeSet registered
	Detach();

	// @gdemers attach actor + add AttributeSet
	AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	OwningOuter = Parent;

	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->RegisterAttributeSet(OwnedAttributeSet, this);
	}
}

void ATriggeringAttachmentActor::Detach()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Detaching \"%s\" from Outer \"%s\" at SocketName \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *ATriggeringAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName(),
	       *SocketName.ToString());

	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

	// @gdemers clear AttributeSet provided by this attachment.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		// @gdemers violating const_ness to ensure user dont do weird shit on cached item elsewhere.
		ASC->UnRegisterAttributeSet(this);
	}
}

UAbilitySystemComponent* ATriggeringAttachmentActor::GetAbilitySystemComponent() const
{
	return UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
}

void ATriggeringAttachmentActor::SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet)
{
	OwnedAttributeSet = NewAttributeSet;
}

TInstancedStruct<FAVVMSocketTargetingHelper> ATriggeringAttachmentActor::GetSocketHelper_Implementation() const
{
	return FAVVMSocketTargetingHelper::Make<FTriggeringAttachmentSocketTargetingHelper>();
}

void ATriggeringAttachmentActor::DeferredSocketParenting_Implementation(AActor* Dest)
{
	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Dest);

	const bool bDoesImplement = UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	IAVVMDoesSupportSocketDeferral::FOnParentSocketAvailableDelegate::FDelegate Callback;
	Callback.BindUObject(this, &ATriggeringAttachmentActor::OnSocketParentingDeferred);
	DeferredSocketParentingDelegateHandle = SocketDeferral->OnSocketParentAvailableDelegate_Add(Callback);
}

void ATriggeringAttachmentActor::OnSocketParentingDeferred(AActor* Parent)
{
	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Parent);

	const bool bDoesImplement = UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	SocketDeferral->OnSocketParentAvailableDelegate_Remove(DeferredSocketParentingDelegateHandle);
	AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
}
