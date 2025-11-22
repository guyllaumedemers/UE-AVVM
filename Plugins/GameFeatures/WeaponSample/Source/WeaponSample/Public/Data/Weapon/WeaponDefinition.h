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
#include "Data/TriggeringDefinitionDataAsset.h"

#include "WeaponDefinition.generated.h"

class UCurveFloat;

/**
 *	Class description:
 *	
 *	F3CModifier is a context struct that encapsulate 3c states modifier specific to the owner Pawn.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API F3CModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Idle = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Walk = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Crouch = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Sprint = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Slide = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Jump = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float Fall = 0.f;
};

/**
 *	Class description:
 *	
 *	FGameplayModifier is a context struct that encapsulate gameplay states modifier specific to the owner Pawn.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FGameplayModifier
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float HealthNonCritical = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float HealthCritical = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float HealthFull = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float StaminaNonCritical = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float StaminaCritical = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float StaminaFull = 0.f;
};

/**
 *	Class description:
 *	
 *	FSpreadCeiling is a context struct that encapsulate ceiling information (clamp) about the spread property.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FSpreadCeiling
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	F3CModifier WhileMoving = F3CModifier();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float WhileFiring = 1.f;
};

/**
 *	Class description:
 *	
 *	FSpreadProperties is a context struct that encapsulate all data relevant to spread calculation.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FSpreadProperties
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	F3CModifier ModifiersWhenMoving = F3CModifier();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	F3CModifier ModifiersWhenFiring = F3CModifier();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayModifier ModifiersWhenUnderCondition = FGameplayModifier();

	// @gdemers to modify the spread speed based on 3c, run calculation such as : Rate * ModifiersWhenMoving.Walk
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Increment by which we scale over time."))
	float Rate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FSpreadCeiling Ceiling = FSpreadCeiling();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float DelayBeforeDecreaseRate = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float DecreaseRate = 0.f;

	// @gdemers sample a point during tick on the curve based on properties here
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UCurveFloat> Curve = nullptr;
};

/**
 *	Class description:
 *	
 *	FRecoilProperties is a context struct that encapsulate all data relevant to recoil calculation.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FRecoilProperties
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	F3CModifier ModifiersWhenMoving = F3CModifier();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	F3CModifier ModifiersWhenFiring = F3CModifier();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayModifier ModifiersWhenUnderCondition = FGameplayModifier();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FVector2D Rate_XY = FVector2D::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float RateModifier_ADS = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float RateModifier_Hip = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FVector2D Ceiling = FVector2D::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float DelayBeforeDecreaseRate = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float CancellationThreshold = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float DecreaseRate = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UCurveFloat> Curve_X = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UCurveFloat> Curve_Y = nullptr;
};

/**
 *	Class description:
 *	
 *	FAmmunitionProperties is a context struct that encapsulate properties for a weapon ammunition capacity.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FAmmunitionProperties
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Maximum amount of ammunition allowed in Clip or Chamber."))
	int32 ReloadCeiling = 0;

	// @gdemers if shell insertion, reload time will be for single shell, and the total would
	// be defined by : MaxCapacity * ShellOrClipReloadTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float ShellOrClipReloadTime = 0.f;
};

/**
 *	Class description:
 *	
 *	FWeaponRange_Properties is a context struct that encapsulate properties of a range weapon.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FWeaponRange_Properties : public FTriggeringProperties
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Slots referenced in the loadout system."))
	FGameplayTagContainer SlotTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PrimaryFiringMode = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bDoesSupportAlternateFiringMode = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesSupportAlternateFiringMode"))
	FGameplayTag SecondaryFiringMode = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FAmmunitionProperties AmmunitionProperties = FAmmunitionProperties();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FRecoilProperties RecoilProperties = FRecoilProperties();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FSpreadProperties SpreadProperties = FSpreadProperties();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bDoesSupportAccurateFirstShot = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesSupportAccurateFirstShot"))
	float TimeUntilFirstShotReset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	float RateOfFire = 0.f;
};

/**
 *	Class description:
 *	
 *	FWeaponMelee_Properties is a context struct that encapsulate properties of a melee weapon.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FWeaponMelee_Properties : public FTriggeringProperties
{
	GENERATED_BODY()
	
	// TODO @gdemers define!
};

/**
 *	Class description:
 *	
 *	UWeaponUtils is a utility class for reusable calculation. 
 */
UCLASS()
class WEAPONSAMPLE_API UWeaponUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Weapon|Utils")
	static void GetAccumulatedRecoil(const ACharacter* Character,
	                                 const float DeltaTime,
	                                 FVector2D& OutResult);

	UFUNCTION(BlueprintCallable, Category="Weapon|Utils")
	static void GetAccumulatedSpread(const ACharacter* Character,
	                                 const float DeltaTime,
	                                 float& OutResult);

	UFUNCTION(BlueprintCallable, Category="Weapon|Utils")
	static FVector2D GetNormalizedRecoil(const FRecoilProperties& RecoilProperties,
	                                     const FVector2D& AccumulatedRecoil);

	UFUNCTION(BlueprintCallable, Category="Weapon|Utils")
	static float GetNormalizedSpread(const FSpreadProperties& SpreadProperties,
	                                 const float AccumulatedSpread);
};
