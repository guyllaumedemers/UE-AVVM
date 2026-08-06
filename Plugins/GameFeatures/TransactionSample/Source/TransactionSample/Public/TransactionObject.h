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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "TransactionFactoryUtils.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#include "TransactionObject.generated.h"

/**
 *	Class description:
 *
 *	ETransactionType describe the transaction UObject category. Useful for filtering a collection.
 */
UENUM(BlueprintType)
enum class ETransactionType : uint8
{
	None,
	Damage,
	Healing,
	Death,
	Raise,
	Kill,
	Killstreak,
	Assist,
	Xp,
	Currency,
	Combo,
	Max
};

inline const TCHAR* EnumToString(ETransactionType State)
{
	switch (State)
	{
		case ETransactionType::Damage:
			return TEXT("Damage");
		case ETransactionType::Healing:
			return TEXT("Healing");
		case ETransactionType::Death:
			return TEXT("Death");
		case ETransactionType::Raise:
			return TEXT("Raise");
		case ETransactionType::Kill:
			return TEXT("Kill");
		case ETransactionType::Killstreak:
			return TEXT("Killstreak");
		case ETransactionType::Assist:
			return TEXT("Assist");
		case ETransactionType::Xp:
			return TEXT("Xp");
		case ETransactionType::Currency:
			return TEXT("Currency");
		case ETransactionType::Combo:
			return TEXT("Combo");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *	
 *	FTransactionObject is a generic type that reference the payload of a gameplay event owned
 *	by a PlayerState.
 */
USTRUCT(BlueprintType)
struct TRANSACTIONSAMPLE_API FTransactionObject : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	bool operator==(const FTransactionObject& Rhs) const;
	
	void PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer);

	// @gdemers he who triggered/caused this transaction event.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString InstigatorId = FString();

	// @gdemers he who owns this transaction.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString TargetId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	ETransactionType TransactionType = ETransactionType::None;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload = FString();
};

/**
 *	Class description:
 *	
 *	FTransactionObjectFastArray is a FastArraySerializer derived class that pack data sequentially, and
 *	is optimized for network serialization.
 */
USTRUCT(BlueprintType)
struct TRANSACTIONSAMPLE_API FTransactionObjectFastArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FIrisFastArraySerializer::FastArrayDeltaSerialize<FTransactionObject, FTransactionObjectFastArray>(TransactionObjects, DeltaParms, *this);
	}

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FTransactionObject> TransactionObjects;
};

template <>
struct TStructOpsTypeTraits<FTransactionObjectFastArray> : public TStructOpsTypeTraitsBase2<FTransactionObjectFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/**
 *	Class description:
 *	
 *	UTransactionObjectUtils is a blueprint function library that expose reusable api.
 */
UCLASS()
class TRANSACTIONSAMPLE_API UTransactionObjectUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static bool DoesExactMatch(const FTransactionObject& NewTransactionObject,
	                           const FString& NewTargetId,
	                           const ETransactionType NewTransactionType);

	UFUNCTION(BlueprintCallable)
	static bool DoesPartialMatch(const FTransactionObject& NewTransactionObject,
	                             const FString& NewTargetId);

	UFUNCTION(BlueprintCallable)
	static FString ToString(const FTransactionObject& NewTransactionObject);

	UFUNCTION(BlueprintCallable)
	static TInstancedStruct<FTransactionPayload> GetValue(const FTransactionObject& NewTransactionObject);

	UFUNCTION(BlueprintCallable)
	static FString GetUniqueId(const AActor* NewTarget);

	UFUNCTION(BlueprintCallable)
	static FTransactionObject MakeTransaction(const AActor* NewInstigator,
	                                          const AActor* NewTarget,
	                                          const ETransactionType NewTransactionType,
	                                          const FString& NewPayload);
};
