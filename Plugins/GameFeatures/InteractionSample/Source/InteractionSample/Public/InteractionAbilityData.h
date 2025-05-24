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

#include "CommonUserWidget.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "InteractionAbilityData.generated.h"

/**
 *	Class Description :
 *
 *	FInteractionAbilityTransitionDetails describe the properties that define the interaction transition
 *	once the activation phase is complete.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionAbilityTransitionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bDisplayContentInWorld = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UDataAsset> CameraTransitionVfx = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;
};

/**
 *	Class Description :
 *
 *	FInteractionAbilityActivationDetails describe the properties that define the interaction.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionAbilityActivationInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bCanInterruptActivation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bHasActivationDelay = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(EditCondition="bHasActivationDelay"))
	float ActivationDelay = 0.f;

	// @gdemers display visual feedback that we are in range for interaction.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UCommonUserWidget> PreviewInteractionWidgetClass = nullptr;
};

/**
 *	Class Description :
 *
 *	UInteractionAbilityDataAsset encapsulate an interaction properties that define how the interaction should be executed.
 */
UCLASS(BlueprintType, NotBlueprintable)
class INTERACTIONSAMPLE_API UInteractionAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable)
	const FInteractionAbilityActivationInfo& GetActivationInfo() const;

	UFUNCTION(BlueprintCallable)
	const FInteractionAbilityTransitionInfo& GetTransitionInfo() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FInteractionAbilityActivationInfo ActivationInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FInteractionAbilityTransitionInfo TransitionInfo;
};
