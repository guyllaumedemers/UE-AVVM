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

#include "AVVMExecutionContextParams.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMExecutionContextRule.generated.h"

/**
 *	Class description:
 *
 *	FAVVMExecutionContextRule is a context struct that define the conditions required for executing an action.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMExecutionContextRule
{
	GENERATED_BODY()

	virtual ~FAVVMExecutionContextRule() = default;
	virtual bool Predicate(const UObject* WorldContextObject,
	                       const TInstancedStruct<FAVVMExecutionContextParams>& Params) const PURE_VIRTUAL(Predicate, return false;);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FAVVMExecutionContextRule>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FAVVMExecutionContextRule> Make(TArgs&&... Args);
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FAVVMExecutionContextRule> FAVVMExecutionContextRule::Make(TArgs&&... Args)
{
	return TInstancedStruct<FAVVMExecutionContextRule>::Make<TChild>(Forward<TArgs>(Args)...);
}

template <>
struct TBaseStructure<FAVVMExecutionContextRule>
{
	static AVVMGAMEPLAY_API UScriptStruct* Get();
};

/**
 *	Class description:
 *	
 *	UAVVMExecutionContextUtils expose a set of utility function relevant for evaluating event execution conditions.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMExecutionContextUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static bool CanExecute(const UObject* WorldContextObject,
	                       const TInstancedStruct<FAVVMExecutionContextParams>& Params,
	                       const TInstancedStruct<FAVVMExecutionContextRule>& Rule);
};
