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

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS
#include "UObject/Object.h"

#include "Transaction.generated.h"

/**
 *	Class description:
 *
 *	ETransactionType describe the transaction UObject category. Useful for filtering
 *	a collection.
 */
UENUM(BlueprintType)
enum class ETransactionType : uint8
{
	None,
	DamageDealt,
	Healing,
	Death,
	Raise,
	Kill,
	Killstreak,
	Assist,
	EarnedCurrency,
	SpentCurrency,
	Max
};

inline const TCHAR* EnumToString(ETransactionType State)
{
	switch (State)
	{
		case ETransactionType::DamageDealt:
			return TEXT("DamageDealt");
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
		case ETransactionType::EarnedCurrency:
			return TEXT("EarnedCurrency");
		case ETransactionType::SpentCurrency:
			return TEXT("SpentCurrency");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	UTransaction are replicated UObject that store replicated data.
 */
UCLASS(BlueprintType)
class TRANSACTIONSAMPLE_API UTransaction : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	UFUNCTION(BlueprintCallable)
	bool DoesMatch(const FString& NewOwnerId, const ETransactionType NewTransactionType) const;
	
	UFUNCTION(BlueprintCallable)
	FString ToString() const;

protected:
	UPROPERTY(Transient, BlueprintReadWrite, Replicated)
	FString OwnerId = FString();

	UPROPERTY(Transient, BlueprintReadWrite, Replicated)
	ETransactionType TransactionType = ETransactionType::None;

	UPROPERTY(Transient, BlueprintReadWrite, Replicated)
	FString Payload = FString();

private:
	void operator()(const FString& NewOwnerId,
	                const ETransactionType NewTransactionType,
	                const FString& NewPayload);

	friend class ATransactionHistory;
};
