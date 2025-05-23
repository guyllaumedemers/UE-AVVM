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

#include "AVVMNotificationSubsystem.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AVVMCheatData.generated.h"

/**
 *	Class description:
 *
 *	FAVVMCheatData. Base Class for "stub" data. Expected to be derived and extended based on project requirements.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMCheatData : public FAVVMNotificationPayload
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	UAVVMCheatDataAsset is binary asset type which define "stub" information about a running cheat. To be loaded/accessed during
 *	Cheats execution.
 *
 *	Benefit: UDataAsset, allow Data inheritance implying that design could make Data-Only BP of this Class and
 *	provide Context specific inputs to parent properties.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVM_API UAVVMCheatDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	const TInstancedStruct<FAVVMCheatData>& GetData() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TInstancedStruct<FAVVMCheatData> CheatData;
};

/**
 *	Class description:
 *
 *	FAVVMCheatDataTableRow. Struct Class referenced in UDataTable and accessed via a UDataRegistry. It's expected
 *	to be retrieved through UMetaDataRegistrySources_DataTable at Runtime.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMCheatDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// @gdemers softptr to the resource asset that contain data specific to the payload
	// we expect the CheatExtension to be provided with.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAVVMCheatDataAsset> CheatDataAsset = nullptr;
};
