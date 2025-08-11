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
#include "AVVMOnlineInterfaceUtils.h"

#include "AVVMOnlineInterface.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

bool UAVVMOnlineInterfaceUtils::IsHosting(const FUniqueNetIdPtr PlayerUniqueNetIdPtr,
                                          const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface)
{
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(OnlineInterface);
	if (bIsValid)
	{
		return OnlineInterface->IsHosting(PlayerUniqueNetIdPtr);
	}
	else
	{
		return false;
	}
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlineIdentityInterface(const UObject* DerivedChild, TScriptInterface<IAVVMOnlineIdentityInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlineIdentityInterface,
	                    IAVVMOnlineIdentityInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlinePartyInterface(const UObject* DerivedChild,
                                                             TScriptInterface<IAVVMOnlinePartyInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlinePartyInterface,
	                    IAVVMOnlinePartyInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlineFriendInterface(const UObject* DerivedChild, TScriptInterface<IAVVMOnlineFriendInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlineFriendInterface,
	                    IAVVMOnlineFriendInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlineMessagingInterface(const UObject* DerivedChild, TScriptInterface<IAVVMOnlineMessagingInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlineMessagingInterface,
	                    IAVVMOnlineMessagingInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlineChallengesInterface(const UObject* DerivedChild,
                                                                  TScriptInterface<IAVVMOnlineChallengesInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlineChallengesInterface,
	                    IAVVMOnlineChallengesInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlineBattlePassInterface(const UObject* DerivedChild,
                                                                  TScriptInterface<IAVVMOnlineBattlePassInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlineBattlePassInterface,
	                    IAVVMOnlineBattlePassInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::GetOuterOnlineStoreInterface(const UObject* DerivedChild,
                                                             TScriptInterface<IAVVMOnlineStoreInterface>& OutInterface)
{
	return GetInterface<UAVVMOnlineStoreInterface,
	                    IAVVMOnlineStoreInterface>(DerivedChild, OutInterface);
}

bool UAVVMOnlineInterfaceUtils::IsFirstPlayerHosting(const UObject* WorldContextObject,
                                                     const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	const ULocalPlayer* Player = IsValid(GameInstance) ? GameInstance->GetFirstGamePlayer() : nullptr;
	const FUniqueNetIdPtr UniqueNetIdPtr = UAVVMOnlineInterfaceUtils::GetUniqueNetIdPtr(Player);
	return IsHosting(UniqueNetIdPtr, OnlineInterface);
}

FUniqueNetIdPtr UAVVMOnlineInterfaceUtils::GetUniqueNetIdPtr(const ULocalPlayer* Player)
{
	FUniqueNetIdPtr UniqueNetIdPtr;
	if (IsValid(Player))
	{
		UniqueNetIdPtr = Player->GetUniqueNetIdForPlatformUser().GetV1();
	}

	return UniqueNetIdPtr;
}

FString UAVVMOnlineInterfaceUtils::SerializePlayerWallet(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMPlayerWalletProxy>(Payload);
}

FString UAVVMOnlineInterfaceUtils::SerializePlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMPlayerProfileProxy>(Payload);
}

FString UAVVMOnlineInterfaceUtils::SerializeHostConfiguration(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMHostConfigurationProxy>(Payload);
}

FString UAVVMOnlineInterfaceUtils::SerializeParty(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMPartyProxy>(Payload);
}

FString UAVVMOnlineInterfaceUtils::SerializePlayerConnection(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMPlayerConnectionProxy>(Payload);
}

FString UAVVMOnlineInterfaceUtils::SerializePlayerChallenge(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMPlayerChallenge>(Payload);
}

FString UAVVMOnlineInterfaceUtils::SerializePlayerResource(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	return SerializeToString<FAVVMPlayerResource>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetPlayerWallet(const FString& Payload)
{
	return DeserializeString<FAVVMPlayerWalletProxy>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetPlayerProfile(const FString& Payload)
{
	return DeserializeString<FAVVMPlayerProfileProxy>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetHostConfiguration(const FString& Payload)
{
	return DeserializeString<FAVVMHostConfigurationProxy>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetParty(const FString& Payload)
{
	return DeserializeString<FAVVMPartyProxy>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetPlayerConnection(const FString& Payload)
{
	return DeserializeString<FAVVMPlayerConnectionProxy>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetPlayerChallenge(const FString& Payload)
{
	return DeserializeString<FAVVMPlayerChallenge>(Payload);
}

TInstancedStruct<FAVVMNotificationPayload> UAVVMOnlineInterfaceUtils::GetPlayerResource(const FString& Payload)
{
	return DeserializeString<FAVVMPlayerResource>(Payload);
}
