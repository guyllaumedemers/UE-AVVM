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

#include "AVVMOnlineInterface.h"

#include "AVVMOnlineStringParser.generated.h"

/**
 *	Class description:
 *
 *	UAVVMOnlineInterface is the interface to be implemented by your parser system. It defines the api for general use case
 *	with Struct Types defined here.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineStringParser : public UObject
{
	GENERATED_BODY()

public:
	void FromString(const FString& NewPayload, FAVVMCurrency& OutCurrency) const;
	void ToString(const FAVVMCurrency& NewCurrency, FString& OutFormat) const;
	
	void FromString(const FString& NewPayload, FAVVMPlayerWallet& OutPlayerWallet) const;
	void ToString(const FAVVMPlayerWallet& NewPlayerWallet, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerProfile& OutPlayerProfile) const;
	void ToString(const FAVVMPlayerProfile& NewPlayerProfile, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerAccount& OutPlayerAccount) const;
	void ToString(const FAVVMPlayerAccount& NewPlayerAccount, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMHostConfiguration& OutHostConfiguration) const;
	void ToString(const FAVVMHostConfiguration& NewHostConfiguration, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMParty>& OutParties) const;
	void ToString(const TArray<FAVVMParty>& NewParties, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMParty& OutParty) const;
	void ToString(const FAVVMParty& NewParty, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerConnection>& OutPlayerConnections) const;
	void ToString(const TArray<FAVVMPlayerConnection>& NewPlayerConnections, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerConnection& OutPlayerConnection) const;
	void ToString(const FAVVMPlayerConnection& NewPlayerConnection, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerChallenge>& OutPlayerChallenges) const;
	void ToString(const TArray<FAVVMPlayerChallenge>& NewPlayerChallenges, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerChallenge& OutPlayerChallenge) const;
	void ToString(const FAVVMPlayerChallenge& NewPlayerChallenge, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerResource>& OutPlayerResources) const;
	void ToString(const TArray<FAVVMPlayerResource>& NewPlayerResources, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerResource& OutPlayerResource) const;
	void ToString(const FAVVMPlayerResource& NewPlayerResource, FString& OutFormat) const;
};
