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

#include "AVVMGameplayTagStackObject.h"
#include "GameFramework/OnlineReplStructs.h"

#include "FastTransactionObject.generated.h"

/**
 *	Class description:
 *	
 *	FLightweightTransactionObject is a FFastArraySerializerItem that identify a gameplay event, stat, score, etc... via tag, and predict server
 *	updates.
 *	
 *	It encapsulates all transaction information about a player.
 */
USTRUCT(BlueprintType)
struct TRANSACTIONSAMPLE_API FLightweightTransactionObject : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLightweightTransactionObject() = default;
	FLightweightTransactionObject(const FLightweightTransactionObject&) = default;
	FLightweightTransactionObject(FLightweightTransactionObject&&) noexcept = default;
	FLightweightTransactionObject& operator=(const FLightweightTransactionObject&) = default;
	FLightweightTransactionObject& operator=(FLightweightTransactionObject&&) noexcept = default;
	bool operator==(const FLightweightTransactionObject& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	FAVVMGameplayTagStackObjectFastArray PlayerTransactions{};
};

/**
 *	Class description:
 *	
 *	FLightweightTransactionObjectFastArray is a FastArraySerializer derived class that pack data sequentially, and
 *	is optimized for network serialization.
 */
USTRUCT(BlueprintType)
struct TRANSACTIONSAMPLE_API FLightweightTransactionObjectFastArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FIrisFastArraySerializer::FastArrayDeltaSerialize<FLightweightTransactionObject, FLightweightTransactionObjectFastArray>(PerPlayerTransactions, DeltaParams, *this);
	}

	int32 Modify(const FUniqueNetIdRepl& PlayerUniqueId,
	             const FGameplayTag& Tag,
	             const int32 DeltaValue/* +/- */,
	             const bool bHasAuthority);

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FLightweightTransactionObject> PerPlayerTransactions{};

	// @gdemers fast access lookup into the replicated array of a player transactions
	TMap<FUniqueNetIdRepl, FLightweightTransactionObject*> FastLookup{};
};
