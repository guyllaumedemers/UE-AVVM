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

#include "ExecutionContextParams.generated.h"

class UItemObject;

/**
 *	Class description:
 *
 *	FExecutionContextParams is a context struct that defines the properties to be
 *	involved in executing an action, as well as the execution implementation details itself.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FExecutionContextParams
{
	GENERATED_BODY()

	virtual ~FExecutionContextParams() = default;

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FExecutionContextParams>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FExecutionContextParams> Make(TArgs&&... Args);

	static TInstancedStruct<FExecutionContextParams> Empty;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FExecutionContextParams> FExecutionContextParams::Make(TArgs&&... Args)
{
	return TInstancedStruct<FExecutionContextParams>::Make<TChild>(Forward<TArgs>(Args)...);
}

template <>
struct TBaseStructure<FExecutionContextParams>
{
	static INVENTORYSAMPLE_API UScriptStruct* Get();
};

/**
 *	Class description:
 *
 *	FDropContextParams is a context struct that defines the properties to be
 *	involved in executing a drop action.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FDropContextParams : public FExecutionContextParams
{
	GENERATED_BODY()

	FDropContextParams() = default;
	FDropContextParams(UItemObject* NewItemObject);

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UItemObject> ItemObject = nullptr;
};

/**
 *	Class description:
 *
 *	FPickupContextParams is a context struct that defines the properties to be
 *	involved in executing a pickup action.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FPickupContextParams : public FExecutionContextParams
{
	GENERATED_BODY()

	FPickupContextParams() = default;
	FPickupContextParams(UItemObject* NewItemObject);

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UItemObject> ItemObject = nullptr;
};

/**
 *	Class description:
 *
 *	FSwapContextParams is a context struct that defines the properties to be
 *	involved in executing a swap action.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FSwapContextParams : public FExecutionContextParams
{
	GENERATED_BODY()

	FSwapContextParams() = default;
	FSwapContextParams(UItemObject* NewSrcItemObject, UItemObject* NewDestItemObject);

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UItemObject> SrcItemObject = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UItemObject> DestItemObject = nullptr;
};
