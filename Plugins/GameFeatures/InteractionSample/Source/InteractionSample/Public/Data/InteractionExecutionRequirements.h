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

#include "StructUtils/InstancedStruct.h"

#include "InteractionExecutionRequirements.generated.h"

class UActorInteractionImpl;
class UInputAction;

/**
 *	Class description:
 *
 *	FInteractionExecutionRequirements is a POD that hold the data to validate prior to interaction execution.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionExecutionRequirements
{
	GENERATED_BODY()

	virtual ~FInteractionExecutionRequirements() = default;
	virtual bool DoesMeetRequirements(const UActorInteractionImpl* Impl) const PURE_VIRTUAL(DoesMetRequirements, return false;);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FInteractionExecutionRequirements>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FInteractionExecutionRequirements> Make(TArgs&&... Args);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FInteractionExecutionRequirements> FInteractionExecutionRequirements::Make(TArgs&&... Args)
{
	return TInstancedStruct<FInteractionExecutionRequirements>::Make<TChild>(Forward<TArgs>(Args)...);
}

/**
*	Class description:
 *
 *	FInteractionExecutionFloatRequirements expose float value requirements for interaction execution.
 */
USTRUCT()
struct INTERACTIONSAMPLE_API FInteractionExecutionFloatRequirements : public FInteractionExecutionRequirements
{
	GENERATED_BODY()

	FInteractionExecutionFloatRequirements() = default;
	FInteractionExecutionFloatRequirements(const float NewThreshold);
	virtual bool DoesMeetRequirements(const UActorInteractionImpl* Impl) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="!bRequireInputMashing"))
	bool bRequireInputHolding = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="!bRequireInputHolding"))
	bool bRequireInputMashing = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CompletionThreshold = 0.f;
};
