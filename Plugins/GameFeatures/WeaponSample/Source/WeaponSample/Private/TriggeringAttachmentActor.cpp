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

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AttachmentManagerComponent.h"
#include "AVVMGameplayUtils.h"
#include "GameplayEffect.h"
#include "WeaponSample.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"

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

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		auto* AttachmentManagerComponent = UAttachmentManagerComponent::GetActorComponent(Outer);
		if (IsValid(AttachmentManagerComponent))
		{
			FGetAttachmentModifierDefinitionRequestArgs Args;
			Args.AttachmentActor = this;
			Args.AttachmentModifierDefinitionId = AttachmentModifierDefinitionId;
			AttachmentManagerComponent->GetAttachmentModifierDefinition(Args);
		}
	}
#endif
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

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		UnRegisterGameplayEffects();
	}
#endif
}

void ATriggeringAttachmentActor::Attach()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Attaching \"%s\" to Outer \"%s\" at SocketName \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *ATriggeringAttachmentActor::StaticClass()->GetName(),
	       *Outer->GetName(),
	       *SocketName.ToString());

	// @gdemers exception case in which I violate const-ness on purpose. theres no reason why the attachment should modify
	// the owning type other than during attach/detach behaviour or when adding itself as children to the Owner child list for replication.
	// In my opinion, the engine didnt set properly their access specifier. We modify the parent RootComponent, not the actor.
	AttachToActor(const_cast<AActor*>(Outer), FAttachmentTransformRules::KeepRelativeTransform, SocketName);
}

void ATriggeringAttachmentActor::Detach()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
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
}

void ATriggeringAttachmentActor::RegisterGameplayEffects(UAbilitySystemComponent* NewAbilitySystemComponent,
                                                         const TArray<UObject*>& NewResources)
{
	if (!ensureAlwaysMsgf(IsValid(NewAbilitySystemComponent), TEXT("Outer didn't return a valid Ability System Component!")))
	{
		return;
	}

	UnRegisterGameplayEffects();

	GameplayEffectSpecHandles.Reset(NewResources.Num());
	for (UObject* Resource : NewResources)
	{
		const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(Cast<UClass>(Resource), this, const_cast<AActor*>(OwningOuter.Get()));
		GameplayEffectSpecHandles.Add(NewAbilitySystemComponent->BP_ApplyGameplayEffectSpecToSelf(GESpecHandle));
	}
}

void ATriggeringAttachmentActor::UnRegisterGameplayEffects()
{
	if (GameplayEffectSpecHandles.IsEmpty())
	{
		return;
	}

	auto* NewAbilitySystemComponent = GameplayEffectSpecHandles[0].GetOwningAbilitySystemComponent();
	if (!ensureAlwaysMsgf(IsValid(NewAbilitySystemComponent), TEXT("The GameplayEffectSpecHandle didn't reference a valid Ability System Component!")))
	{
		return;
	}

	for (const FActiveGameplayEffectHandle& ActiveGameplayEffect : GameplayEffectSpecHandles)
	{
		NewAbilitySystemComponent->RemoveActiveGameplayEffect(ActiveGameplayEffect);
	}
}
