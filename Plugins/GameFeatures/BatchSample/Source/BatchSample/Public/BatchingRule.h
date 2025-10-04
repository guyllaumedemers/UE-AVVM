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

#include "BatchingRule.generated.h"

/**
 *	Class description:
 *
 *	UBatchingRule is a Rule referenced in AVVMWorldSettings. This system defines requirements, and
 *	configure the system Runtime behaviour.
 */
UCLASS(Blueprintable)
class BATCHSAMPLE_API UBatchingRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	bool DoesQualifyForBatchDestroy(const AActor* Actor) const;
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxSizePerBatchDestroy() const;
	
	UFUNCTION(BlueprintCallable)
	float GetBatchInterval() const;
	
	UFUNCTION(BlueprintCallable)
	float GetMaxLifetimeAllowedToUndersizeBatch() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsBatchDestroyEnabled() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bEnableBatchDestroy = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Flag allowing user to define only the Base Class subject to Batch Destroy. Derived Classes with be automatically considered as acceptable for Batch Destroy."))
	bool bAllowBatchDestroyChildClasses = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bEnableBatchDestroy", ToolTip="Base and Derived Classes subject to BatchDestroy."))
	TArray<TSubclassOf<AActor>> Classes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bEnableBatchDestroy", ToolTip="Classes excluded from BatchDestroy."))
	TArray<TSubclassOf<AActor>> IgnoredClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bEnableBatchDestroy"))
	int32 MaxSizePerBatchDestroy = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bEnableBatchDestroy"))
	float IntervalBetweenBatchDestroy = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bEnableBatchDestroy"))
	float MaxLifetimeAllowedToUndersizeBatch = 0.f;
};
