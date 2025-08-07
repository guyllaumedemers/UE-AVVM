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
#include "GameFramework/Actor.h"

void ATriggeringAttachmentActor::BeginPlay()
{
	Super::BeginPlay();

	auto* Outer = GetTypedOuter<AActor>();
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
		auto* AttachementManagerComponent = UAttachmentManagerComponent::GetActorComponent(Outer);
		if (IsValid(AttachementManagerComponent))
		{
			FGetAttachmentModifierDefinitionRequestArgs Args;
			Args.AttachmentActor = this;
			Args.AttachmentModifierDefinitionId = AttachmentModifierDefinitionId;
			AttachementManagerComponent->GetAttachmentModifierDefinition(Args);
		}
	}
#endif
}

void ATriggeringAttachmentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	AActor* Outer = OwningOuter.Get();
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

void ATriggeringAttachmentActor::RegisterGameplayEffects(UAbilitySystemComponent* NewAbilitySystemComponent,
                                                         const TArray<UObject*>& NewResources)
{
	if (!IsValid(NewAbilitySystemComponent))
	{
		return;
	}

	UnRegisterGameplayEffects();

	GameplayEffectSpecHandles.Reset(NewResources.Num());
	for (UObject* Resource : NewResources)
	{
		const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(Cast<UClass>(Resource), this, OwningOuter.Get());
		GameplayEffectSpecHandles.Add(NewAbilitySystemComponent->BP_ApplyGameplayEffectSpecToSelf(GESpecHandle));
	}
}

void ATriggeringAttachmentActor::UnRegisterGameplayEffects()
{
	if (GameplayEffectSpecHandles.IsEmpty())
	{
		return;
	}

	auto* ASC = GameplayEffectSpecHandles[0].GetOwningAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	for (FActiveGameplayEffectHandle& ActiveGameplayEffect : GameplayEffectSpecHandles)
	{
		ASC->RemoveActiveGameplayEffect(ActiveGameplayEffect);
	}
}
