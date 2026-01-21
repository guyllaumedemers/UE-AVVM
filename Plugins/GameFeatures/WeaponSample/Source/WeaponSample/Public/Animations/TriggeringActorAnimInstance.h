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

#include "Animation/AnimInstance.h"

#include "TriggeringActorAnimInstance.generated.h"

class ATriggeringActor;

/**
 *	Class description:
 *	
 *	FTriggeringActorStateProperties_TS is a context struct to be updated based on the Triggering Actor status tags,
 *	and allow running AnimInstance logic on worker thread.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FTriggeringActorStatusProperties_TS
{
	GENERATED_BODY()
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsDraw = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsRelaxed = false;
	
	// @gdemers possibly blocking, aiming for gun, or accentuating
	// next input triggering.
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsRaised = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsTriggered = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsExhausted = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsRecharging = false;
};

/**
 *	Class description:
 *	
 *	UTriggeringActorAnimInstance is a derived type specific to our TriggeringActor who's handling state transition through user inputs,
 *	and/or gameplay state changes.
 */
UCLASS()
class WEAPONSAMPLE_API UTriggeringActorAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// @gdemers only update state flags based on events triggered following local client state change or
	// from the replication tags across network on remote clients.
	UFUNCTION()
	void OnTriggeringActorStateTagChanged(const FGameplayTagContainer& NewStateTags);
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FTriggeringActorStatusProperties_TS StatusProperties_TS = FTriggeringActorStatusProperties_TS();
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const ATriggeringActor> OwningActor = nullptr;
};
