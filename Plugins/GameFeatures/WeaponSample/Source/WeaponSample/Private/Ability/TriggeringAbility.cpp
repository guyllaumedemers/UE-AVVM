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
#include "Ability/TriggerAbility.h"

#include "AVVMLogger.h"
#include "TriggeringActor.h"
#include "WeaponSampleModule.h"
#include "Ability/AVVMGameplayAbilityActorInfo.h"
#include "GameFramework/PlayerController.h"

void UTriggerAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	const AActor* Outer = (ActorInfo != nullptr) ? ActorInfo->OwnerActor.Get() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                Outer,
	                this,
	                TEXT("%s Ability Granted."),
	                *GetName());
}

void UTriggerAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	const AActor* Outer = (ActorInfo != nullptr) ? ActorInfo->OwnerActor.Get() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                Outer,
	                this,
	                TEXT("%s Ability Revoked."),
	                *GetName());
}

bool UTriggerAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags,
                                         FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UTriggerAbility FGameplayAbilityActorInfo invalid!")))
	{
		return false;
	}

	// @gdemers required to pass the internal call to ShouldActivateAbility(AvatarActor->GetLocalRole()) since our ASC is owned
	// by the player state which is simulated_proxy on client.
	FAVVMGameplayAbilityActorInfo ModifiedActorInfo(*ActorInfo);
	return Super::CanActivateAbility(Handle, &ModifiedActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UTriggerAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* Controller = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(Controller),
	                      TEXT("UTriggerAbility PlayerController invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                Controller,
	                this,
	                TEXT("TryActivate %s."),
	                *GetName());

	// @gdemers CommitAbility should consume whatever cost activating this ability required. This imply :
	// ammunition, mana, resources, etc...
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

void UTriggerAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle,
                                    const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                    bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	const APlayerController* PC = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("Abort %s."),
	                *GetName());
}

void UTriggerAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 bool bReplicateEndAbility,
                                 bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UTriggerAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle,
                                    const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                    FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags) ||
		!ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	const auto* GASpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!ensureAlwaysMsgf(GASpec != nullptr, TEXT("Failed to Find Ability Spec from Handle")))
	{
		return false;
	}

	auto* TriggeringActor = Cast<ATriggeringActor>(GASpec->SourceObject);
	if (!ensureAlwaysMsgf(IsValid(TriggeringActor), TEXT("Invalid TriggeringActor.")))
	{
		return false;
	}

	const APlayerController* PC = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("Commit %s."),
	                *GetName());

	TriggeringActor->Trigger();
	return true;
}
