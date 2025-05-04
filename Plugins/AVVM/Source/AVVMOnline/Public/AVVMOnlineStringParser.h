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
#include "UObject/Interface.h"

#include "AVVMOnlineStringParser.generated.h"

/**
 *	Class description:
 *
 *	UAVVMOnlineInterface abstract project parser from the plugin.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineStringParser : public UInterface
{
	GENERATED_BODY()
};

class AVVMONLINE_API IAVVMOnlineStringParser
{
	GENERATED_BODY()

public:
	virtual void FromString(const FString& Payload,
	                        FAVVMPlayerWallet& OutPlayerWallet)
	{
	};

	virtual void ToString(const FAVVMPlayerWallet& PlayerWallet,
	                      FString& OutFormat)
	{
	};

	virtual void FromString(const FString& Payload,
	                        FAVVMPlayerProfile& OutPlayerProfile)
	{
	};

	virtual void ToString(const FAVVMPlayerProfile& PlayerProfile,
	                      FString& OutFormat)
	{
	};

	virtual void FromString(const FString& Payload,
	                        FAVVMHostConfiguration& OutHostConfiguration)
	{
	};

	virtual void ToString(const FAVVMHostConfiguration& HostConfiguration,
	                      FString& OutFormat)
	{
	};

	virtual void FromString(const FAVVMStringPayload& Payload,
	                        TArray<FAVVMParty>& OutParties)
	{
	};

	virtual void FromString(const FString& Payload,
	                        FAVVMParty& OutParty)
	{
	};

	virtual void ToString(const FAVVMParty& Party,
	                      FString& OutFormat)
	{
	};

	virtual void FromString(const FAVVMStringPayload& Payload,
	                        TArray<FAVVMPlayerConnection>& OutPlayerConnections)
	{
	};

	virtual void FromString(const FString& Payload,
	                        FAVVMPlayerConnection& OutPlayerConnection)
	{
	};

	virtual void ToString(const FAVVMPlayerConnection& PlayerConnection,
	                      FString& OutFormat)
	{
	};

	virtual void FromString(const FAVVMStringPayload& Payload,
	                        TArray<FAVVMRuntimeChallenge>& OutPlayerChallenges)
	{
	};

	virtual void FromString(const FString& Payload,
	                        FAVVMRuntimeChallenge& OutPlayerChallenge)
	{
	};

	virtual void ToString(const FAVVMRuntimeChallenge& PlayerChallenge,
	                      FString& OutFormat)
	{
	};

	virtual void FromString(const FAVVMStringPayload& Payload,
	                        TArray<FAVVMRuntimeResource>& OutPlayerResources)
	{
	};

	virtual void FromString(const FString& Payload,
	                        FAVVMRuntimeResource& OutPlayerResource)
	{
	};

	virtual void ToString(const FAVVMRuntimeResource& PlayerResource,
	                      FString& OutFormat)
	{
	};
};
