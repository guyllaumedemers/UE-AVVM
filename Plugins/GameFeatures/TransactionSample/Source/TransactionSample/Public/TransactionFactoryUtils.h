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
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"

#include "TransactionFactoryUtils.generated.h"

/**
 *	Class description:
 *
 *	FTransactionPayload is the Payload base class for caching data specific to gameplay. Each Transaction Type should
 *	have their own derived type. Users can store information about gameplay statistics here.
 *
 *	How to use it ?
 *
 *		* Derive type should define a Constructor with arguments specific to the listed properties.
 *		* Properties defined in the derived type will be listed in the ToString function and display values.
 *
 *	Note : When receiving gameplay events, the derived type can be constructed directly from the constructor, only post-replication does the factory function have to be invoked to convert
 *	from a string and back to an instanced type. (If data has to be red)
 */
USTRUCT(BlueprintType)
struct TRANSACTIONSAMPLE_API FTransactionPayload
{
	GENERATED_BODY()

	virtual ~FTransactionPayload() = default;
	virtual FString ToString() const PURE_VIRTUAL(ToString, return FString(););

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FTransactionPayload>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FTransactionPayload> Make(TArgs&&... Args);

	static TInstancedStruct<FTransactionPayload> Empty;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FTransactionPayload> FTransactionPayload::Make(TArgs&&... Args)
{
	return TInstancedStruct<FTransactionPayload>::Make<TChild>(Forward<TArgs>(Args)...);
}

/**
 *	Class description:
 *
 *	UTransactionFactoryImpl is a Factory base class that generate Payload information specific to your project. Each Transaction Type should
 *	map to a derived class and generate an empty representation of a Payload instance.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class TRANSACTIONSAMPLE_API UTransactionFactoryImpl : public UObject
{
	GENERATED_BODY()

public:
	virtual TInstancedStruct<FTransactionPayload> CreatePayload(const FString& NewPayload) const PURE_VIRTUAL(CreatePayload, return FTransactionPayload::Empty;);
};

/**
 *	Class description:
 *
 *	UTransactionFactoryUtils is a Factory class that expose to BP methods from which a Payload context can be created from or converted
 *	into a String representation.
 */
UCLASS()
class TRANSACTIONSAMPLE_API UTransactionFactoryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static TInstancedStruct<FTransactionPayload> CreatePayloadFromString(const TSubclassOf<UTransactionFactoryImpl>& NewFactoryImpl,
	                                                                     const FString& NewPayload);

	UFUNCTION(BlueprintCallable)
	static FString CreateStringPayload(const TInstancedStruct<FTransactionPayload>& NewPayload);
};
