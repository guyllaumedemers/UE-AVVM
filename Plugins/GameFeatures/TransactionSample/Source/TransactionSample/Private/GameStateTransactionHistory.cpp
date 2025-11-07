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
#include "GameStateTransactionHistory.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Transaction.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UGameStateTransactionHistory::UGameStateTransactionHistory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UGameStateTransactionHistory::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameStateTransactionHistory, Transactions);
}

void UGameStateTransactionHistory::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AGameStateBase>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UGameStateTransactionHistory to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	OwningOuter = Outer;
}

void UGameStateTransactionHistory::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto Iterator = Transactions.CreateIterator(); Iterator; ++Iterator)
	{
		auto* MutableTransaction = const_cast<UTransaction*>(Iterator->Get());
		RemoveReplicatedSubObject(MutableTransaction);
		Iterator.RemoveCurrentSwap();
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UGameStateTransactionHistory from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());
}

void UGameStateTransactionHistory::Static_CreateAndRecordTransaction(const UObject* WorldContextObject,
                                                                     const FTransactionContextArgs& Args)
{
	auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	if (IsValid(TransactionHistory))
	{
		TransactionHistory->CreateAndRecordTransaction(Args);
	}
}

void UGameStateTransactionHistory::Static_RemoveAllTransactionOfType(const UObject* WorldContextObject,
                                                                     const AActor* NewTarget,
                                                                     const ETransactionType NewTransactionType)
{
	auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	if (IsValid(TransactionHistory))
	{
		TransactionHistory->RemoveAllTransactionOfType(NewTarget, NewTransactionType);
	}
}

void UGameStateTransactionHistory::Static_RemoveAllTransactions(const UObject* WorldContextObject,
                                                                const AActor* NewTarget)
{
	auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	if (IsValid(TransactionHistory))
	{
		TransactionHistory->RemoveAllTransactions(NewTarget);
	}
}

TArray<const UTransaction*> UGameStateTransactionHistory::Static_GetAllTransactionsOfType(const UObject* WorldContextObject,
                                                                                          const FString& NewTargetId,
                                                                                          const ETransactionType TransactionType)
{
	const auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	return IsValid(TransactionHistory) ? TransactionHistory->GetAllTransactionsOfType(NewTargetId, TransactionType) : TArray<const UTransaction*>{};
}

TArray<const UTransaction*> UGameStateTransactionHistory::Static_GetAllTransactions(const UObject* WorldContextObject,
                                                                                    const FString& NewTargetId)
{
	const auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	return IsValid(TransactionHistory) ? TransactionHistory->GetAllTransactions(NewTargetId) : TArray<const UTransaction*>{};
}

UGameStateTransactionHistory* UGameStateTransactionHistory::GetActorComponent(const UObject* WorldContextObject)
{
	static TWeakObjectPtr<UGameStateTransactionHistory> TransactionHistory = nullptr;
	if (TransactionHistory.IsValid())
	{
		return TransactionHistory.Get();
	}

	const AGameStateBase* GameState = UGameplayStatics::GetGameState(WorldContextObject);
	if (IsValid(GameState))
	{
		TransactionHistory = GameState->GetComponentByClass<UGameStateTransactionHistory>();
	}

	return TransactionHistory.Get();
}

void UGameStateTransactionHistory::CreateAndRecordTransaction(const FTransactionContextArgs& Args)
{
#if WITH_SERVER_CODE
	if (!Args.Target.IsValid() || !Args.Target->HasAuthority())
	{
		return;
	}

	UTransaction* Transaction = NewObject<UTransaction>(this);
	Transaction->operator()(Args.Instigator.Get(), Args.Target.Get(), Args.TransactionType, Args.Payload);
	AddReplicatedSubObject(Transaction);
	Transactions.Add(Transaction);

	OnRep_NewTransactionRecorded();
#endif
}

void UGameStateTransactionHistory::RemoveAllTransactionOfType(const AActor* NewTarget,
                                                              const ETransactionType NewTransactionType)
{
#if WITH_SERVER_CODE
	if (!IsValid(NewTarget) || !NewTarget->HasAuthority())
	{
		return;
	}

	const TArray<const UTransaction*> SearchResult = GetAllTransactionsOfType(UTransaction::GetUniqueId(NewTarget), NewTransactionType);
	for (const UTransaction* Transaction : SearchResult)
	{
		RemoveReplicatedSubObject(const_cast<UTransaction*>(Transaction)/*bad but also don't want to allow property being mutable elsewhere*/);
		Transactions.Remove(Transaction);
	}

	OnRep_NewTransactionRecorded();
#endif
}

void UGameStateTransactionHistory::RemoveAllTransactions(const AActor* NewTarget)
{
#if WITH_SERVER_CODE
	if (!IsValid(NewTarget) || !NewTarget->HasAuthority())
	{
		return;
	}

	const TArray<const UTransaction*> SearchResult = GetAllTransactions(UTransaction::GetUniqueId(NewTarget));
	for (const UTransaction* Transaction : SearchResult)
	{
		RemoveReplicatedSubObject(const_cast<UTransaction*>(Transaction)/*bad but also don't want to allow property being mutable elsewhere*/);
		Transactions.Remove(Transaction);
	}

	OnRep_NewTransactionRecorded();
#endif
}

TArray<const UTransaction*> UGameStateTransactionHistory::GetAllTransactionsOfType(const FString& NewTargetId,
                                                                                   const ETransactionType TransactionType) const
{
	TArray<TObjectPtr<const UTransaction>> SearchResult = Transactions.FilterByPredicate([NewTargetId, TransactionType](const TObjectPtr<const UTransaction>& Param)
	{
		return IsValid(Param) && Param->DoesExactMatch(NewTargetId, TransactionType);
	});

	TArray<const UTransaction*> Out;
	for (const TObjectPtr<const UTransaction>& Transaction : SearchResult)
	{
		Out.Add(Transaction.Get());
	}

	return Out;
}

TArray<const UTransaction*> UGameStateTransactionHistory::GetAllTransactions(const FString& NewTargetId) const
{
	TArray<TObjectPtr<const UTransaction>> SearchResult = Transactions.FilterByPredicate([NewTargetId](const TObjectPtr<const UTransaction>& Param)
	{
		return IsValid(Param) && Param->DoesPartialMatch(NewTargetId);
	});

	TArray<const UTransaction*> Out;
	for (const TObjectPtr<const UTransaction>& Transaction : SearchResult)
	{
		Out.Add(Transaction.Get());
	}

	return Out;
}

void UGameStateTransactionHistory::OnRep_NewTransactionRecorded()
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". New Transaction Detected!"),
	       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData());

	const TObjectPtr<const UTransaction> NewTransaction = Transactions.IsEmpty() ? nullptr : Transactions.Top();
	TransactionRecordedDelegate.Broadcast(NewTransaction);
}
