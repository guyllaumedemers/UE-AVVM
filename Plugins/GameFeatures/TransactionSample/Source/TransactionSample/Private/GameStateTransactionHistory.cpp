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

#include "AVVMLogger.h"
#include "TransactionObject.h"
#include "TransactionSampleModule.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UGameStateTransactionHistory::UGameStateTransactionHistory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = true;
}

void UGameStateTransactionHistory::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameStateTransactionHistory, Transactions, Params);
}

void UGameStateTransactionHistory::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AGameStateBase>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogTransactionSample,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(UGameStateTransactionHistory::StaticClass()));

	OwningOuter = Outer;
}

void UGameStateTransactionHistory::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Transactions.TransactionObjects.Reset();

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogTransactionSample,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UGameStateTransactionHistory::StaticClass()));
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

TArray<const FTransactionObject*> UGameStateTransactionHistory::Static_GetAllTransactionsOfType(const UObject* WorldContextObject,
                                                                                                const FString& NewTargetId,
                                                                                                const ETransactionType TransactionType)
{
	const auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	return IsValid(TransactionHistory) ? TransactionHistory->GetAllTransactionsOfType(NewTargetId, TransactionType) : TArray<const FTransactionObject*>{};
}

TArray<const FTransactionObject*> UGameStateTransactionHistory::Static_GetAllTransactions(const UObject* WorldContextObject,
                                                                                          const FString& NewTargetId)
{
	const auto* TransactionHistory = UGameStateTransactionHistory::GetActorComponent(WorldContextObject);
	return IsValid(TransactionHistory) ? TransactionHistory->GetAllTransactions(NewTargetId) : TArray<const FTransactionObject*>{};
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

	FTransactionObject Transaction = UTransactionObjectUtils::MakeTransaction(Args.Instigator.Get(), Args.Target.Get(), Args.TransactionType, Args.Payload);
	Transactions.MarkArrayDirty();
	Transactions.TransactionObjects.Add(MoveTemp(Transaction));

	const auto* Outer = OwningOuter.Get();
	if (ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		AVVM_LOGGER_LOG(LogTransactionSample,
		                Outer,
		                Outer,
		                TEXT("Creating new Record %s."),
		                *UTransactionObjectUtils::ToString(Transaction));
	}
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

	Transactions.MarkArrayDirty();
	for (const auto* Transaction : GetAllTransactionsOfType(UTransactionObjectUtils::GetUniqueId(NewTarget), NewTransactionType))
	{
		if (ensureAlwaysMsgf(Transaction != nullptr, TEXT("Invalid Memory access.")))
		{
			Transactions.TransactionObjects.RemoveSingleSwap(*Transaction);
		}
	}

	const auto* Outer = OwningOuter.Get();
	if (ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		AVVM_LOGGER_LOG(LogTransactionSample,
		                Outer,
		                Outer,
		                TEXT("Remove all transactions of type %s from %s."),
		                EnumToString(NewTransactionType),
		                *GetNameSafe(NewTarget));
	}
#endif
}

void UGameStateTransactionHistory::RemoveAllTransactions(const AActor* NewTarget)
{
#if WITH_SERVER_CODE
	if (!IsValid(NewTarget) || !NewTarget->HasAuthority())
	{
		return;
	}

	Transactions.MarkArrayDirty();
	for (const auto* Transaction : GetAllTransactions(UTransactionObjectUtils::GetUniqueId(NewTarget)))
	{
		if (ensureAlwaysMsgf(Transaction != nullptr, TEXT("Invalid Memory access.")))
		{
			Transactions.TransactionObjects.RemoveSingleSwap(*Transaction);
		}
	}

	const auto* Outer = OwningOuter.Get();
	if (ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		AVVM_LOGGER_LOG(LogTransactionSample,
		                Outer,
		                Outer,
		                TEXT("Remove all transactions from %s."),
		                *GetNameSafe(NewTarget));
	}
#endif
}

TArray<const FTransactionObject*> UGameStateTransactionHistory::GetAllTransactionsOfType(const FString& NewTargetId,
                                                                                         const ETransactionType TransactionType) const
{
	TArray<const FTransactionObject*> OutResult;
	for (const auto& TransactionObject : Transactions.TransactionObjects)
	{
		if (UTransactionObjectUtils::DoesExactMatch(TransactionObject, NewTargetId, TransactionType))
		{
			OutResult.Add(&TransactionObject);
		}
	}

	return OutResult;
}

TArray<const FTransactionObject*> UGameStateTransactionHistory::GetAllTransactions(const FString& NewTargetId) const
{
	TArray<const FTransactionObject*> OutResult;
	for (const auto& TransactionObject : Transactions.TransactionObjects)
	{
		if (UTransactionObjectUtils::DoesPartialMatch(TransactionObject, NewTargetId))
		{
			OutResult.Add(&TransactionObject);
		}
	}

	return OutResult;
}
