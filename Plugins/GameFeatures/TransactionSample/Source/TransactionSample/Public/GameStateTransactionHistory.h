﻿//Copyright(c) 2025 gdemers
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
 *	During gameplay, it aggregate statistics for later access and display with UI or third party service.
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
	void CreateAndRecordTransaction(const FString& NewOwnerId,
	                                const ETransactionType NewTransactionType,
	                                const FString& NewPayload);

	// @gdemers no type conversion supported to return const TArray<const UTransaction*>, so we cant support UFUNCTION(BlueprintCallable)
	TArray<TObjectPtr<const UTransaction>> GetTransactions(const FString& OwnerId,
	                                                       const ETransactionType TransactionType) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FOnTransactionRecorded TransactionRecordedDelegate;

protected:
	UFUNCTION()
	void OnRep_NewTransactionRecorded();

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewTransactionRecorded")
	TArray<TObjectPtr<const UTransaction>> Transactions;

	TWeakObjectPtr<const AGameStateBase> OwningOuter = nullptr;
};
