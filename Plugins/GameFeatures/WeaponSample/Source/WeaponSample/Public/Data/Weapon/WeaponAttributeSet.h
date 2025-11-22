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

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

#include "WeaponAttributeSet.generated.h"

class UCurveFloat;

/**
 *	Class description:
 *	
 *	UWeaponRange_AttributeSet is a data type that initialize properties on an Actor ASC for range weapons.
 */
UCLASS(BlueprintType)
class WEAPONSAMPLE_API UWeaponRange_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, TimeUntilFirstShotReset);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, RateOfFire);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, ReloadCeiling);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, ShellOrClipReloadTime);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_MovementPenalityModifier);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_GameplayPenalityModifier);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_RateX);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_RateY);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_RateModifier_ADS);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_RateModifier_Hip);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_CeilingX);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_CeilingY);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_DelayBeforeDecreasing);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_DecreaseRate);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Recoil_DecreaseCancellationThreshold);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_MovementPenalityModifier);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_GameplayPenalityModifier);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_Rate);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_MovementCeil);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_FiringCeil);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_DelayBeforeDecreaseRate);
	ATTRIBUTE_ACCESSORS_BASIC(UWeaponRange_AttributeSet, Spread_DecreaseRate);

protected:
	// ------------------- FAttributeSetProperties ------------------- //
	// @gdemers IMPORTANT : How we initialize our properties 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FAttributeSetProperties")
	TSoftObjectPtr<UDataTable> AttributeMetaDataTable = nullptr;

	// ------------------- FFiringProperties ------------------- //
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FFiringProperties")
	FGameplayAttributeData TimeUntilFirstShotReset = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FFiringProperties")
	FGameplayAttributeData RateOfFire = FGameplayAttributeData();

	// ------------------- FiringMode ------------------- //
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FiringMode", meta=(ToolTip="Slots referenced in the loadout system."))
	FGameplayTagContainer SlotTags = FGameplayTagContainer::EmptyContainer;
	const FGameplayTagContainer& GetSlotTags() const { return SlotTags; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FiringMode")
	FGameplayTag PrimaryFiringMode = FGameplayTag::EmptyTag;
	const FGameplayTag& GetPrimaryFiringMode() const { return PrimaryFiringMode; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FiringMode")
	bool bDoesSupportAlternateFiringMode = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FiringMode", meta=(EditCondition="bDoesSupportAlternateFiringMode"))
	FGameplayTag SecondaryFiringMode = FGameplayTag::EmptyTag;
	const FGameplayTag& GetSecondaryFiringMode() const { return bDoesSupportAlternateFiringMode ? SecondaryFiringMode : FGameplayTag::EmptyTag; }

	// ------------------- FAmmunitionProperties ------------------- //
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FAmmunitionProperties", meta=(ToolTip="Maximum amount of ammunition allowed in Clip or Chamber."))
	FGameplayAttributeData ReloadCeiling = FGameplayAttributeData();

	// @gdemers if shell insertion, reload time will be for single shell, and the total would
	// be defined by : MaxCapacity * ShellOrClipReloadTime;
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FAmmunitionProperties")
	FGameplayAttributeData ShellOrClipReloadTime = FGameplayAttributeData();

	// ------------------- FRecoilProperties ------------------- //
	// @gdemers If you want to modify recoil during free fall or other movement state.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_MovementPenalityModifier = FGameplayAttributeData();

	// @gdemers If you want to modify recoil based on owner gameplay state.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_GameplayPenalityModifier = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_RateX = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_RateY = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_RateModifier_ADS = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_RateModifier_Hip = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_CeilingX = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_CeilingY = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_DelayBeforeDecreasing = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_DecreaseCancellationThreshold = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FRecoilProperties")
	FGameplayAttributeData Recoil_DecreaseRate = FGameplayAttributeData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FRecoilProperties")
	TSoftObjectPtr<UCurveFloat> Recoil_CurveX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FRecoilProperties")
	TSoftObjectPtr<UCurveFloat> Recoil_CurveY = nullptr;

	// ------------------- FSpreadProperties ------------------- //
	// @gdemers If you want to modify spread during free fall or other movement state.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties")
	FGameplayAttributeData Spread_MovementPenalityModifier = FGameplayAttributeData();

	// @gdemers If you want to modify spread based on owner gameplay state.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties")
	FGameplayAttributeData Spread_GameplayPenalityModifier = FGameplayAttributeData();

	// @gdemers to modify the spread speed based on 3c, run calculation such as : Rate * ModifiersWhenMoving.Walk
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties", meta=(ToolTip="Increment by which we scale over time."))
	FGameplayAttributeData Spread_Rate = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties")
	FGameplayAttributeData Spread_MovementCeil = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties")
	FGameplayAttributeData Spread_FiringCeil = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties")
	FGameplayAttributeData Spread_DelayBeforeDecreaseRate = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FSpreadProperties")
	FGameplayAttributeData Spread_DecreaseRate = FGameplayAttributeData();

	// @gdemers sample a point during tick on the curve based on properties here
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FSpreadProperties")
	TSoftObjectPtr<UCurveFloat> Spread_Curve = nullptr;
};

/**
 *	Class description:
 *	
 *	UWeaponMelee_AttributeSet is a data type that initialize properties on an Actor ASC for melee weapons.
 */
UCLASS(BlueprintType)
class WEAPONSAMPLE_API UWeaponMelee_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
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
};
