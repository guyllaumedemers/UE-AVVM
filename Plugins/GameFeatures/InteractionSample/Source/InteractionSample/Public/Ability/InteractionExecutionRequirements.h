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

#include "InteractionExecutionRequirements.generated.h"

class UActorInteractionImpl;

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
	virtual bool DoesMetRequirements(const UActorInteractionImpl* Impl) const PURE_VIRTUAL(DoesMetRequirements, return false;);
};

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
	virtual bool DoesMetRequirements(const UActorInteractionImpl* Impl) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Threshold = 0.f;
};
