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
#include "PlayerInteractionAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"

void UPlayerInteractionAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerInteractionAbility FGameplayAbilityActorInfo invalid!")))
	{
		return;
	}

	const AActor* Outer = ActorInfo->OwnerActor.Get();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Ability Granted \"%s\" on Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *GetName(),
	       *Outer->GetName());
}

void UPlayerInteractionAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UPlayerInteractionAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayTagContainer* SourceTags,
                                                   const FGameplayTagContainer* TargetTags,
                                                   FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UPlayerInteractionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerInteractionAbility FGameplayAbilityActorInfo invalid!")))
	{
		return;
	}

	const AActor* Outer = ActorInfo->OwnerActor.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer),
	                      TEXT("UPlayerInteractionAbility Owning Actor invalid!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Activate Ability \"%s\" on Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *GetName(),
	       *Outer->GetName());

	// const UGameStateInteractionComponent* InteractionComponent = GetLazyComponent();
	// if (!IsValid(InteractionComponent))
	// {
	// 	return;
	// }
	//
	// const AActor* EffectCauser = InteractionComponent->GetGameplayEffectCauser(Outer);
	// if (!IsValid(EffectCauser))
	// {
	// 	return;
	// }
	//
	// UE_LOG(LogGameplay,
	//        Log,
	//        TEXT("Executed from \"%s\". Gameplay Effect Causer \"%s\" from Ability \"%s\"."),
	//        UAVVMGameplayUtils::PrintNetMode(EffectCauser).GetData(),
	//        *EffectCauser->GetName(),
	//        *GetName());
}

void UPlayerInteractionAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           bool bReplicateEndAbility,
                                           bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
