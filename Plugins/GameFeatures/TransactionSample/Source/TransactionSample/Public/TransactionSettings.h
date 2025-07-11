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

#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"

#include "TransactionSettings.generated.h"

enum class ETransactionType : uint8;
class UTransactionFactoryImpl;

/**
 *	Class description:
 *
 *	UTransactionSettings is a developer settings class that expose a mapping of Transaction type to Factory Class
 *	for creating instanced struct from a string payload.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UTransactionSettings"))
class TRANSACTIONSAMPLE_API UTransactionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTransactionSettings();

	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UTransactionFactoryImpl> GetFactoryImpl(const ETransactionType NewType);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config)
	TMap<ETransactionType, TSubclassOf<UTransactionFactoryImpl>> Factories;
};
