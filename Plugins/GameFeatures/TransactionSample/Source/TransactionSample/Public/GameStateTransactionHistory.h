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

#include "GameFramework/Info.h"

#include "GameStateTransactionHistory.generated.h"

class AGameStateBase;
enum class ETransactionType : uint8;
class UTransaction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransactionRecorded, const UTransaction*, Transaction);

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

	UFUNCTION(BlueprintCallable)
	static UGameStateTransactionHistory* GetTransactionHistory(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void CreateAndRecordTransaction(const AActor* NewInstigator /*Statistics Source - from which we generate*/,
	                                const AActor* NewTarget /*Statistics Owner - whom we aggregate for*/,
	                                const ETransactionType NewTransactionType,
	                                const FString& NewPayload);

	UFUNCTION(BlueprintCallable)
	void RemoveAllOfType(const AActor* NewTarget,
	                     const ETransactionType NewTransactionType);

	UFUNCTION(BlueprintCallable)
	void RemoveAll(const AActor* NewTarget);

	// @gdemers no type conversion supported to return const TArray<const UTransaction*>, so we cant support UFUNCTION(BlueprintCallable)
	TArray<TObjectPtr<const UTransaction>> GetAllTransactionsOfType(const FString& NewTargetId,
	                                                                const ETransactionType TransactionType) const;

	TArray<TObjectPtr<const UTransaction>> GetAllTransactions(const FString& NewTargetId) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FOnTransactionRecorded TransactionRecordedDelegate;

protected:
	UFUNCTION()
	void OnRep_NewTransactionRecorded();

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewTransactionRecorded")
	TArray<TObjectPtr<const UTransaction>> Transactions;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AGameStateBase> OwningOuter = nullptr;
};
