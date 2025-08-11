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
#include "InputAction.h"

#include "AVVMAbilityInputAction.generated.h"

/**
 *	Class description:
 *
 *	UAVVMAbilityInputAction extend the base UInputAction and expose an additional Tag to be referenced from UAVVMAbilityInputComponent
 *	when triggering Abilities (see FAbilityTriggerData for overview of Ability System event).
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMAbilityInputAction : public UInputAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	int32 GetInputId() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag AbilityTriggerTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	int32 InputId = INDEX_NONE;

	friend class UAVVMAbilityInputComponent;
};
