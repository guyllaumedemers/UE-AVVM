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

#include "Abilities/GameplayAbility.h"

#include "AVVMGameplayAbility.generated.h"

struct FAVVMGameplayAbilityActorInfo;
class UAVVMAbilityInputAction;

/**
 *	Class description:
 *
 *	UAVVMGameplayAbility is a derived class from UGameplayAbility that define in a dynamic fashion the Input Id value used by the
 *	AbilitySpec for supporting WaitForInputPressed and WaitForInputReleased.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// @gdemers input remapping shouldnt affect the returned value here. Our input id is just a unique
	// identifier to trigger events on the ASC so we can trigger AbilityTask::WaitForInput.
	UFUNCTION(BlueprintCallable)
	int32 GetInputId() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMAbilityInputAction> InputAction = nullptr;

	// @gdemers optional override of the actor info provided by the ASC
	TSharedPtr<FAVVMGameplayAbilityActorInfo> OverrideActorInfo = nullptr;
};
