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
#include "TransactionHistory.h"

#include "Transaction.h"
#include "Net/UnrealNetwork.h"

ATransactionHistory::ATransactionHistory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void ATransactionHistory::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATransactionHistory, Transactions);
}

void ATransactionHistory::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_SERVER_CODE
	for (auto Iterator = Transactions.CreateIterator(); Iterator; ++Iterator)
	{
		auto* MutableTransaction = const_cast<UTransaction*>(Iterator->Get());
		RemoveReplicatedSubObject(MutableTransaction);
		Iterator.RemoveCurrentSwap();
	}
#endif
}

void ATransactionHistory::CreateAndRecordTransaction(const FString& NewOwnerId,
                                                     const ETransactionType NewTransactionType,
                                                     const FString& NewPayload)
{
#if WITH_SERVER_CODE
	UTransaction* Transaction = NewObject<UTransaction>(this);
	Transaction->operator()(NewOwnerId, NewTransactionType, NewPayload);
	AddReplicatedSubObject(Transaction);
	Transactions.Add(Transaction);
#endif
}

TArray<TObjectPtr<const UTransaction>> ATransactionHistory::GetTransactions(const FString& OwnerId,
                                                                            const ETransactionType TransactionType) const
{
	return Transactions.FilterByPredicate([OwnerId, TransactionType](const TObjectPtr<const UTransaction>& Transaction)
	{
		return IsValid(Transaction) && Transaction->DoesMatch(OwnerId, TransactionType);
	});
}

void ATransactionHistory::OnRep_NewTransactionRecorded()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_NewTransactionRecorded called on client!"));
	const UTransaction* NewTransaction = Transactions.IsEmpty() ? nullptr : Transactions.Last().Get();
	TransactionRecordedDelegate.Broadcast(NewTransaction);
}
