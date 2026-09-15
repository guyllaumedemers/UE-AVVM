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

#include "TriggerAbility.h"

#include "TriggerHoldAbility.generated.h"

class APlayerController;

/**
 *	Class description:
 *	
 *	UTriggerHoldAbility
 */
UCLASS()
class WEAPONSAMPLE_API UTriggerHoldAbility : public UTriggerAbility
{
	GENERATED_BODY()

public:
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle,
							 const FGameplayAbilityActorInfo* ActorInfo,
							 const FGameplayAbilityActivationInfo ActivationInfo,
							 FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
							 const FGameplayEventData* TriggerEventData = nullptr) override;

protected:
	virtual void RunOptionalTask(const FGameplayAbilitySpecHandle Handle,
								 const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnTick(const float NewDelta);

	virtual void Execute(const FGameplayAbilitySpecHandle Handle,
	                     const FGameplayAbilityActorInfo* ActorInfo,
	                     const FGameplayAbilityActivationInfo ActivationInfo) override;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const APlayerController> PlayerController = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 PredictedTriggerCount{INDEX_NONE};

	// @gdemers this rate could represent a charge action, or a rate of fire.
	UPROPERTY(Transient, BlueprintReadOnly)
	float TriggerProgressRate{FLT_MAX};

	// @gdemers example : when the weapon becomes too hot to operate. I think gears of war does that ?
	UPROPERTY(Transient, BlueprintReadOnly)
	float ForceExitHeldTime{FLT_MAX};

	UPROPERTY(Transient, BlueprintReadOnly)
	float CurrHeldTime{0.f};
};
