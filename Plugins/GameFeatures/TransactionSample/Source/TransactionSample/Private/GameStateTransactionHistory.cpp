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
#include "TransactionFactoryUtils.h"
#include "GameFramework/GameStateBase.h"
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

void UGameStateTransactionHistory::CreateAndRecordTransaction(const AActor* NewInstigator,
                                                              const AActor* NewTarget,
                                                              const ETransactionType NewTransactionType,
                                                              const FString& NewPayload)
{
#if WITH_SERVER_CODE
	if (IsValid(NewTarget) && NewTarget->HasAuthority())
	{
		UTransaction* Transaction = NewObject<UTransaction>(this);
		Transaction->operator()(NewInstigator, NewTarget, NewTransactionType, NewPayload);
		AddReplicatedSubObject(Transaction);
		Transactions.Add(Transaction);

		OnRep_NewTransactionRecorded();
	}
#endif
}

TArray<TObjectPtr<const UTransaction>> UGameStateTransactionHistory::GetTransactions(const FString& NewTargetId,
                                                                                     const ETransactionType TransactionType) const
{
	return Transactions.FilterByPredicate([NewTargetId, TransactionType](const TObjectPtr<const UTransaction>& Param)
	{
		return IsValid(Param) && Param->DoesMatch(NewTargetId, TransactionType);
	});
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
