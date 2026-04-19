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

#include "TransactionFactoryUtils.h"

#include "TransactionFactoryImplTest.generated.h"

/**
 *	Class description:
 *
 *	FTransactionPayloadTest is an example Payload derive type with a single property int32.
 */
USTRUCT()
struct TRANSACTIONSAMPLE_API FTransactionPayloadTest : public FTransactionPayload
{
	GENERATED_BODY()

	FTransactionPayloadTest() = default;
	explicit FTransactionPayloadTest(const int32 NewValue);
	virtual FString ToString() const override;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 Value = INDEX_NONE;
};

/**
 *	Class description:
 *
 *	UTransactionFactoryImplTest is an example Factory Payload derive type that instance a FTransactionPayloadTest.
 */
UCLASS(Blueprintable)
class TRANSACTIONSAMPLE_API UTransactionFactoryImplTest : public UTransactionFactoryImpl
{
	GENERATED_BODY()

public:
	virtual TInstancedStruct<FTransactionPayload> CreatePayload(const FString& NewPayload) const override;
};
