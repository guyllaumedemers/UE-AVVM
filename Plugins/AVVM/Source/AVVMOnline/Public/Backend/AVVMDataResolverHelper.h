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

#include "AVVMDataResolverHelper.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMDataResolverHelper is a polymorphic context from which we derive to resolve backend information.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMDataResolverHelper
{
	GENERATED_BODY()

	virtual ~FAVVMDataResolverHelper() = default;

	virtual TArray<int32> GetElementDependencies(const UObject* WorldContextObject,
	                                             const int32 ElementId) const PURE_VIRTUAL(GetElementDependencies, return TArray<int32>{};);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FAVVMDataResolverHelper>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FAVVMDataResolverHelper> Make(TArgs&&... Args);

	static TInstancedStruct<FAVVMDataResolverHelper> Empty;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FAVVMDataResolverHelper> FAVVMDataResolverHelper::Make(TArgs&&... Args)
{
	return TInstancedStruct<FAVVMDataResolverHelper>::Make<TChild>(Forward<TArgs>(Args)...);
}

template <>
struct TBaseStructure<FAVVMDataResolverHelper>
{
	static AVVMONLINE_API UScriptStruct* Get();
};
