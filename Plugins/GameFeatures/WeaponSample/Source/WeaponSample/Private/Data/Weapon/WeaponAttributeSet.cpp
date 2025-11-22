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
#include "Data/Weapon/WeaponAttributeSet.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Net/UnrealNetwork.h"

void UWeaponRange_AttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponRange_AttributeSet, TimeUntilFirstShotReset);
	DOREPLIFETIME(UWeaponRange_AttributeSet, RateOfFire);
	DOREPLIFETIME(UWeaponRange_AttributeSet, ReloadCeiling);
	DOREPLIFETIME(UWeaponRange_AttributeSet, ShellOrClipReloadTime);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_MovementPenalityModifier);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_GameplayPenalityModifier);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_RateX);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_RateY);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_RateModifier_ADS);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_RateModifier_Hip);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_CeilingX);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_CeilingY);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_DelayBeforeDecreasing);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_DecreaseRate);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Recoil_DecreaseCancellationThreshold);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_MovementPenalityModifier);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_GameplayPenalityModifier);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_Rate);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_MovementCeil);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_FiringCeil);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_DelayBeforeDecreaseRate);
	DOREPLIFETIME(UWeaponRange_AttributeSet, Spread_DecreaseRate);
}

void UWeaponRange_AttributeSet::Init()
{
	// @gdemers IMPORTANT : initialize all properties based on FAttributeMetaDataTable.
	Super::Init();

	// @gdemers Curve access can be executed from SoftObjectPtr, and return valid object. We cache our handle in any case which
	// should prevent gc.
	TArray<FSoftObjectPath> CurveTableSoftObjectPaths;
	CurveTableSoftObjectPaths.Add(Recoil_CurveX.ToSoftObjectPath());
	CurveTableSoftObjectPaths.Add(Recoil_CurveY.ToSoftObjectPath());
	CurveTableSoftObjectPaths.Add(Spread_Curve.ToSoftObjectPath());
	CurveTableHandle = UAssetManager::Get().LoadAssetList(CurveTableSoftObjectPaths, FStreamableDelegate{});
}
