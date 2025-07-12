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

void UPlayerInteractionAbilityBase::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilityActivationInfo ActivationInfo,
                                                FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerInteractionAbility FGameplayAbilityActorInfo invalid!")))
	{
		return;
	}

	const AActor* EffectCauser = GetEffectCauser(ActorInfo->AbilitySystemComponent.Get());
	if (IsValid(EffectCauser))
	{
		TargetComponent = EffectCauser->GetComponentByClass<UActorInteractionComponent>();
	}
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

	const UActorInteractionComponent* InteractionComponent = TargetComponent.Get();
	if (!IsValid(InteractionComponent))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

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

void UPlayerInteractionAbilityBase::CancelAbility(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                                  bool bReplicateCancelAbility)
{
	const APlayerController* PC = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Aborting Ability \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(PC).GetData(),
	       *GetClass()->GetName());

	const UActorInteractionComponent* InteractionComponent = TargetComponent.Get();
	if (IsValid(InteractionComponent))
	{
		InteractionComponent->Kill(PC);
	}

	// @gdemers calls EndAbility which clear TargetComponent otherwise
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
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

bool UPlayerInteractionAbilityBase::CommitAbility(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                                  FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	const APlayerController* PC = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Commiting Ability \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(PC).GetData(),
	       *GetClass()->GetName());

	const UActorInteractionComponent* InteractionComponent = TargetComponent.Get();
	if (IsValid(InteractionComponent))
	{
		InteractionComponent->Execute(PC);
	}

	return true;
}

void UPlayerInteractionAbilityBase::RunOptionalTask(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	const bool bWasCommitted = CommitAbility(Handle, ActorInfo, ActivationInfo);
	if (bWasCommitted)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

const AActor* UPlayerInteractionAbilityBase::GetEffectCauser(const UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!IsValid(AbilitySystemComponent))
	{
		return nullptr;
	}

	const auto GEQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(GEQueryTags);
	const TArray<FActiveGameplayEffectHandle> GEActiveHandles = AbilitySystemComponent->GetActiveGameplayEffects().GetActiveEffects(GEQuery);
	if (GEActiveHandles.IsEmpty())
	{
		return nullptr;
	}

	ensureAlwaysMsgf(GEActiveHandles.Num() == 1, TEXT("Multiple Matches Found! There should only ever be one match!"));

	const FActiveGameplayEffect* GEActive = AbilitySystemComponent->GetActiveGameplayEffect(GEActiveHandles[0]);
	if (GEActive == nullptr)
	{
		return nullptr;
	}

	const FGameplayEffectContextHandle& GECtx = GEActive->Spec.GetEffectContext();
	return GECtx.GetEffectCauser();
}
