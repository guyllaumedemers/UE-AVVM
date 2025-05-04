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
#include "Kismet/GameplayStatics.h"

bool UAVVMOnlineInterfaceUtils::IsHosting(const FUniqueNetIdPtr PlayerUniqueNetIdPtr,
                                          const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface)
{
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(OnlineInterface);
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

ULocalPlayer* UAVVMOnlineInterfaceUtils::GetFirstLocalPlayer(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (IsValid(GameInstance))
	{
		return GameInstance->GetFirstGamePlayer();
	}
	else
	{
		return nullptr;
	}
}
