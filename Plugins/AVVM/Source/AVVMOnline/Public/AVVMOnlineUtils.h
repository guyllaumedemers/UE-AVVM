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

#include "AVVMNotificationSubsystem.h"
#include "AVVMOnline.h"
#include "AVVMOnlinePlayerStringParser.h"
#include "AVVMUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online/CoreOnlineFwd.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMOnlineUtils.generated.h"

class IAVVMOnlineBattlePassInterface;
class IAVVMOnlineChallengesInterface;
class IAVVMOnlineIdentityInterface;
class IAVVMOnlinePartyInterface;
class ULocalPlayer;

/**
 *	Class description:
 *
 *	UAVVMOnlineInterfaceUtils expose a set of utility function relevant for the online service.
 */
UCLASS()
class AVVMONLINE_API UAVVMOnlineUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool IsFirstPlayerHosting(const UObject* WorldContextObject,
									 const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface);

	static bool IsHosting(const FUniqueNetIdPtr PlayerUniqueNetIdPtr,
						  const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface);

	static FUniqueNetIdPtr GetUniqueNetIdPtr(const ULocalPlayer* Player);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString GetUniqueNetId(const APlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlineIdentityInterface(const UObject* DerivedChild,
	                                            TScriptInterface<IAVVMOnlineIdentityInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlinePartyInterface(const UObject* DerivedChild,
	                                         TScriptInterface<IAVVMOnlinePartyInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlineFriendInterface(const UObject* DerivedChild,
	                                          TScriptInterface<IAVVMOnlineFriendInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlineMessagingInterface(const UObject* DerivedChild,
	                                             TScriptInterface<IAVVMOnlineMessagingInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlineChallengesInterface(const UObject* DerivedChild,
	                                              TScriptInterface<IAVVMOnlineChallengesInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlineBattlePassInterface(const UObject* DerivedChild,
	                                              TScriptInterface<IAVVMOnlineBattlePassInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool GetOuterOnlineStoreInterface(const UObject* DerivedChild,
	                                         TScriptInterface<IAVVMOnlineStoreInterface>& OutInterface);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializePlayerWallet(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializePlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializeHostConfiguration(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializeParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializePlayerConnection(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializePlayerChallenge(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static FString SerializePlayerResource(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetPlayerWallet(const FString& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetPlayerProfile(const FString& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetHostConfiguration(const FString& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetParty(const FString& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetPlayerConnection(const FString& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetPlayerChallenge(const FString& Payload);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TInstancedStruct<FAVVMNotificationPayload> GetPlayerResource(const FString& Payload);

private:
	template <typename UInterfaceClass, typename IInterfaceClass>
	static bool GetInterface(const UObject* DerivedChild,
	                         TScriptInterface<IInterfaceClass>& OutInterface);

	template <typename TPayload>
	static FString SerializeToString(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	template <typename TPayload>
	static TInstancedStruct<FAVVMNotificationPayload> DeserializeString(const FString& Payload);
};

template <typename UInterfaceClass, typename IInterfaceClass>
bool UAVVMOnlineUtils::GetInterface(const UObject* DerivedChild,
                                    TScriptInterface<IInterfaceClass>& OutInterface)
{
	if (!IsValid(DerivedChild))
	{
		return false;
	}

	auto* Outer = Cast<UObject>(DerivedChild->GetImplementingOuterObject(UInterfaceClass::StaticClass()));
	if (!ensureAlways(IsValid(Outer)))
	{
		UE_LOG(LogAVVMOnline,
		       Log,
		       TEXT("%s doesn't Implement %s"),
		       *DerivedChild->GetName(),
		       *UInterfaceClass::StaticClass()->GetName());

		return false;
	}

	OutInterface = TScriptInterface<IInterfaceClass>(Outer);

	const bool bImplement = UAVVMUtils::IsNativeScriptInterfaceValid(OutInterface);
	if (!bImplement)
	{
		UE_LOG(LogAVVMOnline,
		       Log,
		       TEXT("%s is Null. Does %s implement the interface in Blueprint? If so, Update to support the interface Natively!"),
		       *UInterfaceClass::StaticClass()->GetName(),
		       *Outer->GetName());
	}

	return bImplement;
}

template <typename TPayload>
FString UAVVMOnlineUtils::SerializeToString(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	const auto* StringParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(StringParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return FString();
	}

	const auto* Data = Payload.GetPtr<TPayload>();
	if (!ensureAlways(Data != nullptr))
	{
		UE_LOG(LogAVVMOnline,
		       Log,
		       TEXT("TPayload isn't deriving from received type."))

		return FString();
	}

	FString OutFormat;
	StringParser->ToString(*Data, OutFormat);
	return OutFormat;
}

template <typename TPayload>
TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineUtils::DeserializeString(const FString& Payload)
{
	const auto* StringParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(StringParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return FAVVMNotificationPayload::Empty;
	}

	TPayload OutPayload;
	StringParser->FromString(Payload, OutPayload);
	return FAVVMNotificationPayload::Make<TPayload>(OutPayload);
}
