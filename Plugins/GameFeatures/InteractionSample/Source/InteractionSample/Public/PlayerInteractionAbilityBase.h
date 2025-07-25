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

#include "Ability/AVVMGameplayAbility.h"

#include "PlayerInteractionAbilityBase.generated.h"

class UActorInteractionComponent;

/**
 *	Class Description :
 *
 *	UPlayerInteractionAbilityBase is an instance ability that can be invoked through user input when in-range of a world actor
 *	with an UActorInteractionComponent.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class INTERACTIONSAMPLE_API UPlayerInteractionAbilityBase : public UAVVMGameplayAbility
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

	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle,
	                         const FGameplayAbilityActorInfo* ActorInfo,
	                         const FGameplayAbilityActivationInfo ActivationInfo,
	                         FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData = nullptr) override;

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
	virtual void RunOptionalTask(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer GEQueryTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UActorInteractionComponent> TargetComponent = nullptr;
};
