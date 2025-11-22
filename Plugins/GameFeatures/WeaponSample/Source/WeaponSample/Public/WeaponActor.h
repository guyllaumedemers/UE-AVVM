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
#include "TriggeringActor.h"

#include "WeaponActor.generated.h"

class UArrowComponent;
class UAttachmentManagerComponent;
class UProjectileComponent;
class USkeletalMeshComponent;

/**
 *	Class description:
 *	
 *	AWeaponActor_Range is a system actor that handle behaviour specific to a magic weapon.
 */
UCLASS()
class WEAPONSAMPLE_API AWeaponActor_Range : public ATriggeringActor
{
	GENERATED_BODY()

public:
	AWeaponActor_Range(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Trigger_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	void ToggleFiringMode(const FGameplayTag& NewFiringMode);

protected:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttachmentManagerComponent> AttachmentManagerComponent = nullptr;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileComponent> ProjectileComponent = nullptr;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	// @gdemers Arrow held by Outer Actor that is aligned with the ACharacter expected World FTransform
	// when holding this Weapon. Using proxy remove dependency on Animation state being set on the Server.
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UArrowComponent> WeaponProxyComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FGameplayTag CurrentFiringMode = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *	
 *	AWeaponActor_Melee is a system actor that handle behaviour specific to a melee weapon.
 */
UCLASS()
class WEAPONSAMPLE_API AWeaponActor_Melee : public ATriggeringActor
{
	GENERATED_BODY()

public:
	AWeaponActor_Melee(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Trigger_Implementation() const override;

protected:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;
};
