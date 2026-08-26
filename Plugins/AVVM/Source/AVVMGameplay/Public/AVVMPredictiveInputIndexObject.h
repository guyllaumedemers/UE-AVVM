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

#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMPredictiveInputIndexObject.generated.h"

/**
 *	Wrapper macro to register Closure Types with `this` class FAVVMPredictiveInputIndexObject property.
 */
#define BIND_PREDICTED_INPUT_INDEX_CHANGED_CLOSURE_TYPE(WeakFunc)\
{\
	[ThisInstance = TWeakObjectPtr(this)](const int32 PredictedTargetIndex) { return ThisInstance.IsValid() ? ThisInstance->WeakFunc(PredictedTargetIndex) : false; }\
}\

/**
 *	Class description:
 *	
 *	EPredictiveState is an enum class type that represent transient states for a predicted input capture.
 */
UENUM(BlueprintType)
enum class EPredictiveState : uint8
{
	None = 0 UMETA(Hidden),
	Paused,
	Running,
	PendingKill
};

/**
 *	Class description:
 *	
 *	FAVVMPredictiveInputIndexObject is a utility object meant to manage predictive cases where
 *	input captured require tracking invalidated states, and possibly Restore them.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMPredictiveInputIndexObject
{
	GENERATED_BODY()

	FAVVMPredictiveInputIndexObject() = default;
	FAVVMPredictiveInputIndexObject(TFunction<bool(const int32)> OnNewPause,
	                                TFunction<bool(const int32)> OnNewRestore,
	                                TFunction<bool(const int32)> OnNewRestart,
	                                const int32 ReserveArraySize);

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<EPredictiveState> PredictiveInputIndexes{};

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 PrevInputIndex = INDEX_NONE;

	TFunction<bool(const int32)> OnPause{};
	TFunction<bool(const int32)> OnRestore{};
	TFunction<bool(const int32)> OnRestart{};

	friend class UAVVMPredictiveInputUtils;
};

/**
 *	Class description:
 *	
 *	UAVVMPredictiveInputUtils 
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMPredictiveInputUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static bool Capture(const int32 NewInputIndex, FAVVMPredictiveInputIndexObject& OutResult);

	UFUNCTION(BlueprintCallable)
	static bool Flush(FAVVMPredictiveInputIndexObject& OutResult);
};
