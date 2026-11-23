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

#include "TransactionObject.h"
#include "Components/ActorComponent.h"
#include "Payloads/AVVMComplexPayloads.h"
#include "StructUtils/InstancedStruct.h"

#include "TransactionHistoryComponent.generated.h"

class AGameStateBase;
enum class ETransactionType : uint8;

/**
 *	Class description:
 *
 *	FTransactionContextArgs is a context struct that encapsulate function parameters arguments.
 */
USTRUCT(BlueprintType)
struct TRANSACTIONSAMPLE_API FTransactionContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const AActor> Instigator = nullptr; /*Statistics Source - from which we generate*/

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const AActor> Target = nullptr; /*Statistics Owner - whom we aggregate for*/

	UPROPERTY(Transient, BlueprintReadWrite)
	ETransactionType TransactionType{ETransactionType::None};

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload{};
};

/**
 *	Class description:
 *
 *	UTransactionHistoryComponent is a component system that capture gameplay statistics
 *	based on the player gameplay experience.
 *	
 *	Note : Use this system for persistent statistics with complex payload. example : FAVVMJsonPayload.
 */
UCLASS(ClassGroup=("Transaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class TRANSACTIONSAMPLE_API UTransactionHistoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTransactionHistoryComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static void Static_CreateAndRecordTransaction(const UObject* WorldContextObject,
	                                              FTransactionContextArgs Args);

	UFUNCTION(BlueprintCallable)
	static void Static_RemoveAllTransactionOfType(const UObject* WorldContextObject,
	                                              const AActor* NewTarget,
	                                              const ETransactionType NewTransactionType);

	UFUNCTION(BlueprintCallable)
	static void Static_RemoveAllTransactions(const UObject* WorldContextObject,
	                                         const AActor* NewTarget);

	template <typename TValue>
	static void Static_GetAggregatedValues(const UObject* WorldContextObject,
	                                       const FString& NewTargetId,
	                                       const ETransactionType TransactionType,
	                                       TValue& OutResult);

	static TArray<const FTransactionObject*> Static_GetAllTransactionsOfType(const UObject* WorldContextObject,
	                                                                         const FString& NewTargetId,
	                                                                         const ETransactionType TransactionType);

	static TArray<const FTransactionObject*> Static_GetAllTransactions(const UObject* WorldContextObject,
	                                                                   const FString& NewTargetId);

protected:
	static UTransactionHistoryComponent* GetActorComponent(const UObject* WorldContextObject);
	void CreateAndRecordTransaction(FTransactionContextArgs&& Args);
	void RemoveAllTransactionOfType(const AActor* NewTarget, const ETransactionType NewTransactionType);
	void RemoveAllTransactions(const AActor* NewTarget);
	TArray<const FTransactionObject*> GetAllTransactionsOfType(const FString& NewTargetId, const ETransactionType TransactionType) const;
	TArray<const FTransactionObject*> GetAllTransactions(const FString& NewTargetId) const;

	template <typename TValue>
	void GetAggregatedValues(const FString& NewTargetId,
	                         const ETransactionType TransactionType,
	                         TValue& OutResult) const;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FTransactionObjectFastArray Transactions{};

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AGameStateBase> OwningOuter = nullptr;
};

template <typename TValue>
void UTransactionHistoryComponent::Static_GetAggregatedValues(const UObject* WorldContextObject,
                                                              const FString& NewTargetId,
                                                              const ETransactionType TransactionType,
                                                              TValue& OutResult)
{
	const auto* TransactionHistory = UTransactionHistoryComponent::GetActorComponent(WorldContextObject);
	if (IsValid(TransactionHistory))
	{
		TransactionHistory->GetAggregatedValues<TValue>(NewTargetId, TransactionType, OutResult);
	}
}

template <typename TValue>
void UTransactionHistoryComponent::GetAggregatedValues(const FString& NewTargetId,
                                                       const ETransactionType TransactionType,
                                                       TValue& OutResult) const
{
	for (const auto* Transaction : GetAllTransactionsOfType(NewTargetId, TransactionType))
	{
		if (!ensureAlwaysMsgf(Transaction != nullptr, TEXT("Invalid Memory access.")))
		{
			continue;
		}

		const TInstancedStruct<FAVVMNotificationPayload> InstancedPayload = UTransactionObjectUtils::GetValue(*Transaction);
		const auto Value = FAVVMJsonPayload::From<TValue>(InstancedPayload.GetPtr<FAVVMJsonPayload>());
		OutResult += Value;
	}
}
