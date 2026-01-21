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
#include "Animation/AnimInstance.h"

#include "AVVMCharacterAnimInstance.generated.h"

class AAVVMCharacter;

/**
 *	Class description:
 *	
 *	FAVVMMovementProperties_TS is a context struct to be updated based on the AVVMCharacter movement state tags,
 *	and allow running AnimInstance logic on worker thread.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMMovementProperties_TS
{
	GENERATED_BODY()
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsIdle = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsWalking = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsSprinting = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsRunning = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsSliding = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsCharging = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsSwimming = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsGliding = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsClimbing = false;
};

/**
 *	Class description:
 *	
 *	FAVVMStateProperties_TS is a context struct to be updated based on the AVVMCharacter status tags,
 *	and allow running AnimInstance logic on worker thread.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMStatusProperties_TS
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsGrounded = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsAired = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsDown = false;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsRagdoll = false;
};

/**
 *	Class description:
 *	
 *	UAVVMCharacterAnimInstance is a derived type specific to our Character who's handling state transition through user inputs,
 *	and/or gameplay state changes.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMCharacterAnimInstance : public UAnimInstance
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
	void OnCharacterStateTagChanged(const FGameplayTagContainer& NewStateTags);
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FAVVMMovementProperties_TS MovementProperties_TS = FAVVMMovementProperties_TS();
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FAVVMStatusProperties_TS StatusProperties_TS = FAVVMStatusProperties_TS();
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AAVVMCharacter> OwningCharacter = nullptr;
};
