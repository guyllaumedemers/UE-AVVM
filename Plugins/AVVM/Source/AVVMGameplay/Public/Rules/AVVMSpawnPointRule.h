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

#include "AVVMSpawnPointRule.generated.h"

/**
 *	Class description:
 *
 *	UAVVMSpawnPointRetrySearchRule is a UObject instance that attempt retrieval of a valid Start position, and define how failure to find such output
 *	should behave following invocation call to AGameMode::RestartPlayer.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMSpawnPointRetrySearchRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
	// @gdemers validate information about player based on the referenced UniqueNetId.
	UFUNCTION(BlueprintNativeEvent)
	bool Predicate_GetSpawnPoint(const TArray<AActor*>& SpawnPoints, AActor*& OutActor) const;
	virtual bool Predicate_GetSpawnPoint_Implementation(const TArray<AActor*>& SpawnPoints, AActor*& OutActor) const PURE_VIRTUAL(Predicate_GetSpawnPoint_Implementation, return false;);
	
	UFUNCTION(BlueprintCallable)
	bool CanUsePreviousStartPositionOnFailure() const;
	
	UFUNCTION(BlueprintCallable)
	bool CanRetrySearch() const;
	
	UFUNCTION(BlueprintCallable)
	float GetRetryRate() const;
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxNumRetry() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bCanUsePreviousStartPositionOnFailure = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bCanRetrySearch = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bCanRetrySearch", ClampMin="0", ClampMax="4"))
	float RetryRate = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bCanRetrySearch", ClampMin="0", ClampMax="25"))
	int32 MaxNumRetry = INDEX_NONE;
};
