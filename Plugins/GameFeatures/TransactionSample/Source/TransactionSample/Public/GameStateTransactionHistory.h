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
#include "Transaction.h"

#include "GameStateTransactionHistory.generated.h"

class AGameStateBase;
enum class ETransactionType : uint8;
class UTransaction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransactionRecorded, const UTransaction*, Transaction);

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
	ETransactionType TransactionType = ETransactionType::None;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Payload = FString();
};

/**
 *	Class description:
 *
 *	UGameStateTransactionHistory capture UTransaction object. It exists on the AGameStateBase and is pushed via GFP.
 *	During gameplay, it aggregates statistics for later access and display with UI or third party service.
 */
UCLASS(ClassGroup=("Transaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class TRANSACTIONSAMPLE_API UGameStateTransactionHistory : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameStateTransactionHistory(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="TransactionSample")
	static void Static_CreateAndRecordTransaction(const UObject* WorldContextObject,
	                                              const FTransactionContextArgs& Args);

	UFUNCTION(BlueprintCallable, Category="TransactionSample")
	static void Static_RemoveAllTransactionOfType(const UObject* WorldContextObject,
	                                              const AActor* NewTarget,
	                                              const ETransactionType NewTransactionType);

	UFUNCTION(BlueprintCallable, Category="TransactionSample")
	static void Static_RemoveAllTransactions(const UObject* WorldContextObject,
	                                         const AActor* NewTarget);

	// @gdemers TArray cannot cast from TArray<const UTransaction*> to TArray<UTransaction*> which is
	// required for BP support. I prefer ensuring const-ness here.
	static TArray<const UTransaction*> Static_GetAllTransactionsOfType(const UObject* WorldContextObject,
	                                                                   const FString& NewTargetId,
	                                                                   const ETransactionType TransactionType);

	static TArray<const UTransaction*> Static_GetAllTransactions(const UObject* WorldContextObject,
	                                                             const FString& NewTargetId);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FOnTransactionRecorded TransactionRecordedDelegate;

protected:
	static UGameStateTransactionHistory* GetActorComponent(const UObject* WorldContextObject);
	void CreateAndRecordTransaction(const FTransactionContextArgs& Args);
	void RemoveAllTransactionOfType(const AActor* NewTarget, const ETransactionType NewTransactionType);
	void RemoveAllTransactions(const AActor* NewTarget);
	TArray<const UTransaction*> GetAllTransactionsOfType(const FString& NewTargetId, const ETransactionType TransactionType) const;
	TArray<const UTransaction*> GetAllTransactions(const FString& NewTargetId) const;

	UFUNCTION()
	void OnRep_NewTransactionRecorded();

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewTransactionRecorded")
	TArray<TObjectPtr<const UTransaction>> Transactions;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AGameStateBase> OwningOuter = nullptr;
};
