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
#include "Components/ActorComponent.h"

#include "DynamicHitboxComponent.generated.h"

class AAVVMCharacter;
class UPawnMovementComponent;
class UPhysicsAsset;
class USkeletalMeshComponent;

/**
 *	 Description :
 *
 *	 EPhysicState represent the various states we expect to support with Physic Asset overwrites.
 */
UENUM(BlueprintType)
enum class EPhysicState : uint8
{
	Default,
	Small,
	Big,
	VeryBig,
};

/**
 *	 Description :
 *
 *	 UDynamicHitboxComponent is a Component held by the ACharacter that handle overwriting the Physic Asset at Runtime based on character state.
 *	 This allows better control for designer to configure colliders and improve hit resolution during fast movement, and preferable for combat.
 */
UCLASS(ClassGroup=("Weapon"), Blueprintable, meta=(BlueprintSpawnableComponent))
class WEAPONSAMPLE_API UDynamicHitboxComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UDynamicHitboxComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	FGameplayTag GetOuterMovementTag() const;
	EPhysicState GetPhysicState(const FGameplayTag& MovementTag) const;
	USkeletalMeshComponent* GetSkeletalMeshComponent();
	void DeferredPhysicSwap();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TMap<EPhysicState, TObjectPtr<UPhysicsAsset>> PhysicAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TMap<FGameplayTag, EPhysicState> MovementTagToPhysicAssets;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UPawnMovementComponent> MovementComponent = nullptr;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AAVVMCharacter> OuterCharacter = nullptr;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	EPhysicState PreviousState = EPhysicState::Default;
};
