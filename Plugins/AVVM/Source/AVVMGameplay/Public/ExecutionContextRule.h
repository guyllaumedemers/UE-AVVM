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

#include "ExecutionContextParams.h"
#include "StructUtils/InstancedStruct.h"

#include "ExecutionContextRule.generated.h"

class UActorComponent;

/**
*	Class description:
 *
 *	FExecutionContextRule is a context struct that define the conditions required for executing an action.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FExecutionContextRule
{
	GENERATED_BODY()

	virtual ~FExecutionContextRule() = default;
	virtual bool Predicate(const UActorComponent* Component,
	                       const TInstancedStruct<FExecutionContextParams>& Params) const PURE_VIRTUAL(Predicate, return false;);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FExecutionContextRule>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FExecutionContextRule> Make(TArgs&&... Args);
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FExecutionContextRule> FExecutionContextRule::Make(TArgs&&... Args)
{
	return TInstancedStruct<FExecutionContextRule>::Make<TChild>(Forward<TArgs>(Args)...);
}

template <>
struct TBaseStructure<FExecutionContextRule>
{
	static AVVMGAMEPLAY_API UScriptStruct* Get();
};
