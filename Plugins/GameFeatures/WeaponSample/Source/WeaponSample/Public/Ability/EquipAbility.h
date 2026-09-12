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
#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Ability/AVVMGameplayAbility.h"

#include "EquipAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 *	Class description:
 *	
 *	UEquipAbility_Montage is the ability that enables user to play a montage for equipping a weapon/or attachment actor.
 *	
 *	IMPORTANT - I find that most often then not, games that support an unequip animation end up with transition issues. Most popular
 *	game hide the active item instantly, and ONLY play the equip animation of the next weapon to be active which is what we are aiming
 *	for here!
 */
UCLASS()
class WEAPONSAMPLE_API UEquipAbility_Montage : public UAVVMGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilitySpec& Spec) override;

	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                                const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr,
	                                FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle,
	                           const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo,
	                           bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
	                        const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle,
	                           const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo,
	                           FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

protected:
	UFUNCTION()
	void OnMontage_Interrupted();
	
	UFUNCTION()
	void OnMontage_Cancelled();
	
	UFUNCTION()
	void OnMontage_Completed();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag EquipMontageTag{FGameplayTag::EmptyTag};

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> EquippedTriggeringActor = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> AbilityTask_PlayMontage = nullptr;
};
