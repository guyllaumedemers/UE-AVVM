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
#include "Ability/EquipAbility_Montage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AVVMLogger.h"
#include "TriggeringActor.h"
#include "WeaponSampleModule.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMCharacterAttributeSet.h"
#include "Ability/AVVMGameplayAbilityActorInfo.h"
#include "Animation/AnimMontage.h"
#include "Data/Weapon/WeaponAttributeSet.h"
#include "GameFramework/PlayerController.h"

void UEquipAbility_Montage::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
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

void UEquipAbility_Montage::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
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

bool UEquipAbility_Montage::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayTagContainer* SourceTags,
                                               const FGameplayTagContainer* TargetTags,
                                               FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UEquipAbility FGameplayAbilityActorInfo invalid!")))
	{
		return false;
	}

	// @gdemers required to pass the internal call to ShouldActivateAbility(AvatarActor->GetLocalRole()) since our ASC is owned
	// by the player state which is simulated_proxy on client.
	FAVVMGameplayAbilityActorInfo ModifiedActorInfo(*ActorInfo);
	return Super::CanActivateAbility(Handle, &ModifiedActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UEquipAbility_Montage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* Controller = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(Controller),
	                      TEXT("UEquipAbility PlayerController invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                Controller,
	                this,
	                TEXT("TryActivate %s."),
	                *GetName());

	const auto* NextEquipTargetActor = (TriggerEventData != nullptr) ? TriggerEventData->Instigator.Get() : nullptr;
	if (!IsValid(NextEquipTargetActor))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (EquippedTriggeringActor.IsValid() && (NextEquipTargetActor != EquippedTriggeringActor))
	{
		IAVVMDoesActorRequireComplexVisibilitySupport::Execute_ApplyComplexVisibilityToSelf(const_cast<AActor*>(EquippedTriggeringActor.Get()));
		IAVVMDoesActorRequireComplexVisibilitySupport::Execute_ApplyComplexVisibilityToSelf(const_cast<AActor*>(NextEquipTargetActor));
	}
	else if (NextEquipTargetActor != EquippedTriggeringActor)
	{
		IAVVMDoesActorRequireComplexVisibilitySupport::Execute_ApplyComplexVisibilityToSelf(const_cast<AActor*>(NextEquipTargetActor));
	}
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	EquippedTriggeringActor = NextEquipTargetActor;
	// @gdemers cancel any running instance that isnt complete so we can run a montage for the next target.
	if (IsValid(AbilityTask_PlayMontage))
	{
		AbilityTask_PlayMontage->ExternalCancel();
	}

	TSubclassOf<UAnimMontage> SearchResult = IAVVMDoesActorSupportMontages::Execute_GetMontageClassByTag(EquippedTriggeringActor.Get(), EquipMontageTag);
	if (!ensureAlwaysMsgf(IsValid(SearchResult), TEXT("Invalid Montage access.")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	bool bOutResult = false;
	// @gdemers WeaponSample will modify this FGameplayAttribute at runtime based on designer configuration, and/or player progression.
	const float Weapon_EquipPlayRate = UAbilitySystemBlueprintLibrary::GetFloatAttribute(EquippedTriggeringActor.Get(),
	                                                                                     GET_GAMEPLAY_ATTRIBUTE_USING_IILE(UWeaponBase_AttributeSet, EquippedTriggeringActor.Get(), Animation_EquipPlayRate),
	                                                                                     bOutResult);

	ensureAlwaysMsgf(bOutResult, TEXT("Failed to retrieve GameplayAttribute."));
	// @gdemers SkillSample will modify this FGameplayAttribute at runtime based on player Skill Tree configuration.
	const float Player_EquipPlayRateModifier = UAbilitySystemBlueprintLibrary::GetFloatAttribute(ActorInfo->OwnerActor.Get(),
	                                                                                             GET_GAMEPLAY_ATTRIBUTE_USING_IILE(UAVVMCharacterAttributeSet, ActorInfo->OwnerActor.Get(), Animation_EquipPlayRateModifier),
	                                                                                             bOutResult);

	ensureAlwaysMsgf(bOutResult, TEXT("Failed to retrieve GameplayAttribute."));
	AbilityTask_PlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,
	                                                                                         TEXT("EquipOrUnequip_Task"),
	                                                                                         SearchResult->GetDefaultObject<UAnimMontage>(),
	                                                                                         (Weapon_EquipPlayRate * Player_EquipPlayRateModifier),
	                                                                                         NAME_None,
	                                                                                         false/*fire-n-forget montage shouldn't tie its lifecycle to the ability*/,
	                                                                                         1.f,
	                                                                                         0.f,
	                                                                                         false);

	if (IsValid(AbilityTask_PlayMontage))
	{
		AbilityTask_PlayMontage->OnInterrupted.AddUniqueDynamic(this, &UEquipAbility_Montage::OnMontage_Interrupted);
		AbilityTask_PlayMontage->OnCancelled.AddUniqueDynamic(this, &UEquipAbility_Montage::OnMontage_Cancelled);
		AbilityTask_PlayMontage->OnCompleted.AddUniqueDynamic(this, &UEquipAbility_Montage::OnMontage_Completed);
		AbilityTask_PlayMontage->ReadyForActivation();
	}

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

void UEquipAbility_Montage::CancelAbility(const FGameplayAbilitySpecHandle Handle,
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

void UEquipAbility_Montage::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo,
                                       bool bReplicateEndAbility,
                                       bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UEquipAbility_Montage::CommitAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	const APlayerController* PC = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("Commit %s."),
	                *GetName());

	return true;
}

void UEquipAbility_Montage::OnMontage_Interrupted()
{
	const APlayerController* PC = (CurrentActorInfo != nullptr) ? CurrentActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("%s Interrupted."),
	                *GetName());

	K2_CancelAbility();
}

void UEquipAbility_Montage::OnMontage_Cancelled()
{
	const APlayerController* PC = (CurrentActorInfo != nullptr) ? CurrentActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("%s Cancelled."),
	                *GetName());

	K2_CancelAbility();
}

void UEquipAbility_Montage::OnMontage_Completed()
{
	const APlayerController* PC = (CurrentActorInfo != nullptr) ? CurrentActorInfo->PlayerController.Get() : nullptr;
	AVVM_LOGGER_LOG(LogWeaponSample,
	                PC,
	                this,
	                TEXT("%s Complete."),
	                *GetName());
}
