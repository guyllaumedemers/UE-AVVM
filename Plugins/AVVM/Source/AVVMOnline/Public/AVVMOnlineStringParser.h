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

#include "UObject/Object.h"

#include "AVVMOnlineStringParser.generated.h"

struct FAVVMCurrency;
struct FAVVMHostConfiguration;
struct FAVVMParty;
struct FAVVMPartyProxy;
struct FAVVMPlayerAccount;
struct FAVVMPlayerAccountProxy;
struct FAVVMPlayerChallenge;
struct FAVVMPlayerConnection;
struct FAVVMPlayerConnectionProxy;
struct FAVVMPlayerLoginContext;
struct FAVVMPlayerPreset;
struct FAVVMPlayerPresetProxy;
struct FAVVMPlayerProfile;
struct FAVVMPlayerProfileProxy;
struct FAVVMPlayerResource;
struct FAVVMPlayerWallet;
struct FAVVMStringPayload;

/**
 *	Class description:
 *
 *	UAVVMOnlineStringParser is the UObject interfacing with Unreal JSon system to parse the FString payload that defines
 *	the data schema of the project backend.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineStringParser : public UObject
{
	GENERATED_BODY()

public:
	void FromString(const FString& NewPayload, FAVVMPlayerLoginContext& OutPlayerLoginContext) const;
	void ToString(const FAVVMPlayerLoginContext& NewPlayerLoginContext, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerAccount& OutPlayerAccount) const;
	void ToString(const FAVVMPlayerAccount& NewPlayerAccount, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerWallet& OutPlayerWallet) const;
	void ToString(const FAVVMPlayerWallet& NewPlayerWallet, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMCurrency& OutCurrency) const;
	void ToString(const FAVVMCurrency& NewCurrency, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerProfile& OutPlayerProfile) const;
	void ToString(const FAVVMPlayerProfile& NewPlayerProfile, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerPreset& OutPlayerPreset) const;
	void ToString(const FAVVMPlayerPreset& NewPlayerPreset, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerResource>& OutPlayerResources) const;
	void ToString(const TArray<FAVVMPlayerResource>& NewPlayerResources, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerResource& OutPlayerResource) const;
	void ToString(const FAVVMPlayerResource& NewPlayerResource, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerChallenge>& OutPlayerChallenges) const;
	void ToString(const TArray<FAVVMPlayerChallenge>& NewPlayerChallenges, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerChallenge& OutPlayerChallenge) const;
	void ToString(const FAVVMPlayerChallenge& NewPlayerChallenge, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMParty>& OutParties) const;
	void ToString(const TArray<FAVVMParty>& NewParties, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMParty& OutParty) const;
	void ToString(const FAVVMParty& NewParty, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerConnection>& OutPlayerConnections) const;
	void ToString(const TArray<FAVVMPlayerConnection>& NewPlayerConnections, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerConnection& OutPlayerConnection) const;
	void ToString(const FAVVMPlayerConnection& NewPlayerConnection, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMHostConfiguration& OutHostConfiguration) const;
	void ToString(const FAVVMHostConfiguration& NewHostConfiguration, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerAccountProxy& OutPlayerAccountProxy) const;
	void ToString(const FAVVMPlayerAccountProxy& NewPlayerAccountProxy, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerProfileProxy& OutPlayerProfileProxy) const;
	void ToString(const FAVVMPlayerProfileProxy& NewPlayerProfileProxy, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerPresetProxy& OutPlayerPresetProxy) const;
	void ToString(const FAVVMPlayerPresetProxy& NewPlayerPresetProxy, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPartyProxy>& OutPartyProxies) const;
	void ToString(const TArray<FAVVMPartyProxy>& NewPartyProxies, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPartyProxy& OutPartyProxy) const;
	void ToString(const FAVVMPartyProxy& NewPartyProxy, FString& OutFormat) const;

	void FromString(const FAVVMStringPayload& NewPayload, TArray<FAVVMPlayerConnectionProxy>& OutPlayerConnectionProxies) const;
	void ToString(const TArray<FAVVMPlayerConnectionProxy>& NewPlayerConnectionProxies, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMPlayerConnectionProxy& OutPlayerConnectionProxy) const;
	void ToString(const FAVVMPlayerConnectionProxy& NewPlayerConnectionProxy, FString& OutFormat) const;
};
