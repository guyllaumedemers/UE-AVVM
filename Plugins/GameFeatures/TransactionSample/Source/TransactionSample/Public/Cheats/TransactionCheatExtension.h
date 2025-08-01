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

#include "Transaction.h"
#include "GameFramework/CheatManager.h"

#ifdef UE_ENABLE_AVVM_DEBUGGER
#include "AVVMDebugger.h"
#endif

#include "TransactionCheatExtension.generated.h"

/**
 *	Class description:
 *
 *	UTransactionCheatExtension expose basic functionalities for recording UTransaction with the UGameStateTransactionHistoryComponent
 *	during gameplay testing.
 */
UCLASS()
class TRANSACTIONSAMPLE_API UTransactionCheatExtension : public UCheatManagerExtension
#if WITH_AVVM_DEBUGGER
                                                         ,
                                                         public IAVVMImGuiDescriptor
#endif
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	UFUNCTION(Exec, BlueprintCallable, Category="Transaction|Cheats", DisplayName="Transaction.RemoveAllOfType")
	void RemoveAllTransactionsOfType(const ETransactionType NewType, const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="Transaction|Cheats", DisplayName="Transaction.RemoveAll")
	void RemoveAllTransactions(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="Transaction|Cheats", DisplayName="Transaction.Add")
	void AddTransaction(const ETransactionType NewType, const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="Transaction|Cheats", DisplayName="Transaction.PrintAll")
	void PrintAll(const int32 PlayerIndex = 0);

#if WITH_AVVM_DEBUGGER
	virtual void Draw() override;
#endif

protected:
#if WITH_AVVM_DEBUGGER
	const char* LazyGatherTransactionTypes();
#endif
};
