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
#include "PlayerInteractionAbilityBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActorInteractionComponent.h"
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Ability/AVVMGameplayAbilityActorInfo.h"

void UPlayerInteractionAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
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
	       TEXT("Executed from \"%s\". Ability Granted \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *GetName(),
	       *Outer->GetName());
}

void UPlayerInteractionAbilityBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

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
	       TEXT("Executed from \"%s\". Ability Removed \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *GetName(),
	       *Outer->GetName());
}

bool UPlayerInteractionAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo,
                                                       const FGameplayTagContainer* SourceTags,
                                                       const FGameplayTagContainer* TargetTags,
                                                       FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerInteractionAbility FGameplayAbilityActorInfo invalid!")))
	{
		return false;
	}

	// @gdemers required to pass the internal call to ShouldActivateAbility(AvatarActor->GetLocalRole()) since our ASC is owned
	// by the player state which is simulated_proxy on client.
	FAVVMGameplayAbilityActorInfo ModifiedActorInfo(*ActorInfo);
	return Super::CanActivateAbility(Handle, &ModifiedActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UPlayerInteractionAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerInteractionAbility FGameplayAbilityActorInfo invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const AActor* Controller = ActorInfo->PlayerController.Get();
	if (!ensureAlwaysMsgf(IsValid(Controller),
	                      TEXT("UPlayerInteractionAbility PlayerController invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Attempting Ability Activation \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Controller).GetData(),
	       *GetName(),
	       *Controller->GetName());

	const auto* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!IsValid(ASC))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const auto GEQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(GEQueryTags);
	const TArray<FActiveGameplayEffectHandle> GEActiveHandles = ASC->GetActiveGameplayEffects().GetActiveEffects(GEQuery);
	if (GEActiveHandles.IsEmpty())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	ensureAlwaysMsgf(GEActiveHandles.Num() == 1, TEXT("Multiple Matches Found! There should only ever be one match!"));

	const FActiveGameplayEffect* GEActive = ASC->GetActiveGameplayEffect(GEActiveHandles[0]);
	if (GEActive == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const FGameplayEffectContextHandle& GECtx = GEActive->Spec.GetEffectContext();
	const AActor* GEEffectCauser = GECtx.GetEffectCauser();
	if (!IsValid(GEEffectCauser))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	auto* InteractionComponent = GEEffectCauser->GetComponentByClass<UActorInteractionComponent>();
	if (!IsValid(InteractionComponent))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	TargetComponent = InteractionComponent;

	const bool bCanStartExecution = InteractionComponent->StartExecution(Controller);
	if (bCanStartExecution)
	{
		RunOptionalTask(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UPlayerInteractionAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               bool bReplicateEndAbility,
                                               bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	TargetComponent.Reset();
}

void UPlayerInteractionAbilityBase::RunOptionalTask(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	CommitAbility(Handle, ActorInfo, ActivationInfo);
}
