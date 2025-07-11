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

#include "Abilities/Tasks/AbilityTask.h"

#include "AVVMAbilityTask_TickUntil.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityTaskTickDelegate, const float, NewValue);

/**
 *	Class Description :
 *
 *	UAVVMAbilityTask_TickUntil is a tickable task that share the lifetime of it's owning GameplayAbility. Once EndAbility is called on the TaskOwner, the task will be destroyed
 *	and garbage collected.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMAbilityTask_TickUntil : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAVVMAbilityTask_TickUntil(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	static UAVVMAbilityTask_TickUntil* TickUntil(UGameplayAbility* OwningAbility, const bool bTestAlreadyReleased = false);

	virtual void TickTask(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityTaskTickDelegate OnTick;

protected:
	virtual void Activate() override;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bTestInitialState = false;
};
