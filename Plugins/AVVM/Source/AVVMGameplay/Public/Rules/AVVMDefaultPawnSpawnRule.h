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

#include "AVVMWorldSetting.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AVVMDefaultPawnSpawnRule.generated.h"

/**
 *	Class description:
 *
 *	UAVVMDefaultPawnSpawnRule is a UObject instance that define the conditions required for the AGameMode
 *	default Pawn creation.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMDefaultPawnSpawnRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// @gdemers validate this user login conditions for spawning a pawn.
	UFUNCTION(BlueprintNativeEvent)
	bool Predicate_HasMetDefaultPawnSpawnRequirements(const AGameModeBase* GameMode, const APlayerController* PC) const;
	virtual bool Predicate_HasMetDefaultPawnSpawnRequirements_Implementation(const AGameModeBase* GameMode, const APlayerController* PC) const PURE_VIRTUAL(Predicate_HasMetDefaultPawnSpawnRequirements_Implementation, return false;);

	UFUNCTION(BlueprintCallable)
	float GetRetryRate() const;

	UFUNCTION(BlueprintCallable)
	int32 GetMaxNumRetry() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer SpawnConditionTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="0", ClampMax="4"))
	float RetryRate = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="0", ClampMax="25"))
	int32 MaxNumRetry = INDEX_NONE;
};
