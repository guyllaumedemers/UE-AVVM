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
#include "Ability/SwitchTriggerModeAbility_Montage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AVVMLogger.h"
#include "WeaponActor.h"
#include "WeaponSampleModule.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Ability/AVVMCharacterAttributeSet.h"
#include "Ability/AVVMGameplayAbilityActorInfo.h"
#include "Animation/AnimMontage.h"
#include "Data/Weapon/WeaponAttributeSet.h"
#include "GameFramework/PlayerController.h"

void USwitchTriggerModeAbility_Montage::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
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

void USwitchTriggerModeAbility_Montage::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
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

bool USwitchTriggerModeAbility_Montage::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                           const FGameplayAbilityActorInfo* ActorInfo,
                                                           const FGameplayTagContainer* SourceTags,
                                                           const FGameplayTagContainer* TargetTags,
                                                           FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("USwitchTriggerModeAbility_Montage FGameplayAbilityActorInfo invalid!")))
	{
		return false;
	}

	// @gdemers required to pass the internal call to ShouldActivateAbility(AvatarActor->GetLocalRole()) since our ASC is owned
	// by the player state which is simulated_proxy on client.
	FAVVMGameplayAbilityActorInfo ModifiedActorInfo(*ActorInfo);
	return Super::CanActivateAbility(Handle, &ModifiedActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void USwitchTriggerModeAbility_Montage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                        const FGameplayAbilityActorInfo* ActorInfo,
                                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const auto* ASC = (ActorInfo != nullptr) ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(ASC),
	                      TEXT("USwitchTriggerModeAbility_Montage owning ASC invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const auto* GASpec = ASC->FindAbilitySpecFromHandle(Handle);
	if (!ensureAlwaysMsgf(GASpec != nullptr, TEXT("Failed to Find Ability Spec from Handle")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	auto* TriggeringActor = Cast<ATriggeringActor>(GASpec->SourceObject);
	if (!ensureAlwaysMsgf(IsValid(TriggeringActor), TEXT("Invalid TriggeringActor.")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                ActorInfo->PlayerController.Get(),
	                this,
	                TEXT("TryActivate %s."),
	                *GetName());

	EquippedTriggeringActor = TriggeringActor;
	// @gdemers cancel any running instance that isnt complete so we can run a montage for the next target.
	if (IsValid(AbilityTask_PlayMontage))
	{
		AbilityTask_PlayMontage->ExternalCancel();
	}

	TSubclassOf<UAnimMontage> SearchResult = IAVVMDoesActorSupportMontages::Execute_GetMontageClassByTag(EquippedTriggeringActor.Get(), SwitchFiringModeMontageTag);
	if (!ensureAlwaysMsgf(IsValid(SearchResult), TEXT("Invalid Montage access.")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	bool bOutResult = false;
	// @gdemers WeaponSample will modify this FGameplayAttribute at runtime based on designer configuration, and/or player progression.
	const float Weapon_SwitchFiringModePlayRate = UAbilitySystemBlueprintLibrary::GetFloatAttribute(EquippedTriggeringActor.Get(),
	                                                                                                GET_GAMEPLAY_ATTRIBUTE_USING_IILE(UWeaponBase_AttributeSet, EquippedTriggeringActor.Get(), Animation_SwitchFiringModePlayRate),
	                                                                                                bOutResult);

	ensureAlwaysMsgf(bOutResult, TEXT("Failed to retrieve GameplayAttribute."));
	// @gdemers SkillSample will modify this FGameplayAttribute at runtime based on player Skill Tree configuration.
	const float Player_SwitchFiringModePlayRateModifier = UAbilitySystemBlueprintLibrary::GetFloatAttribute(ActorInfo->OwnerActor.Get(),
	                                                                                                        GET_GAMEPLAY_ATTRIBUTE_USING_IILE(UAVVMCharacterAttributeSet, ActorInfo->OwnerActor.Get(), Animation_SwitchFiringModePlayRateModifier),
	                                                                                                        bOutResult);

	ensureAlwaysMsgf(bOutResult, TEXT("Failed to retrieve GameplayAttribute."));
	AbilityTask_PlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,
																							 TEXT("SwitchFiringMode_Montage"),
																							 SearchResult->GetDefaultObject<UAnimMontage>(),
																							 (Weapon_SwitchFiringModePlayRate * Player_SwitchFiringModePlayRateModifier),
																							 NAME_None,
																							 false/*fire-n-forget montage shouldn't tie its lifecycle to the ability*/,
																							 1.f,
																							 0.f,
																							 false);

	if (IsValid(AbilityTask_PlayMontage))
	{
		AbilityTask_PlayMontage->OnInterrupted.AddUniqueDynamic(this, &USwitchTriggerModeAbility_Montage::OnMontage_Interrupted);
		AbilityTask_PlayMontage->OnCancelled.AddUniqueDynamic(this, &USwitchTriggerModeAbility_Montage::OnMontage_Cancelled);
		AbilityTask_PlayMontage->OnCompleted.AddUniqueDynamic(this, &USwitchTriggerModeAbility_Montage::OnMontage_Completed);
		AbilityTask_PlayMontage->ReadyForActivation();
	}
}

void USwitchTriggerModeAbility_Montage::CancelAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (IsValid(AbilityTask_PlayMontage))
	{
		AbilityTask_PlayMontage->ExternalCancel();
	}
}

void USwitchTriggerModeAbility_Montage::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   bool bReplicateEndAbility,
                                                   bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool USwitchTriggerModeAbility_Montage::CommitAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                ActorInfo->PlayerController.Get(),
	                this,
	                TEXT("Commit %s."),
	                *GetName());

	auto* TriggeringActor = Cast<AWeaponActor_Range>(EquippedTriggeringActor.Get());
	if (ensureAlwaysMsgf(IsValid(TriggeringActor), TEXT("Failed to commit on invalid actor.")))
	{
		TriggeringActor->ToggleFiringMode();
	}

	return true;
}

void USwitchTriggerModeAbility_Montage::OnMontage_Interrupted()
{
	const APlayerController* PC = (CurrentActorInfo != nullptr) ? CurrentActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("%s Interrupted."),
	                *GetName());

	K2_CancelAbility();
}

void USwitchTriggerModeAbility_Montage::OnMontage_Cancelled()
{
	const APlayerController* PC = (CurrentActorInfo != nullptr) ? CurrentActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("%s Cancelled."),
	                *GetName());

	K2_CancelAbility();
}

void USwitchTriggerModeAbility_Montage::OnMontage_Completed()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();

	const APlayerController* PC = (CurrentActorInfo != nullptr) ? CurrentActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("%s Complete."),
	                *GetName());

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
