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

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMOnlineInterfaceUtils.generated.h"

class IAVVMOnlineBattlePassInterface;
class IAVVMOnlineChallengesInterface;
class IAVVMOnlineIdentityInterface;
class IAVVMOnlinePartyInterface;
class ULocalPlayer;

/**
 *	Class description:
 *
 *	UAVVMOnlineInterfaceUtils expose a set of utility function defined by the online service.
 */
UCLASS()
class AVVMONLINE_API UAVVMOnlineInterfaceUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static bool GetOuterOnlineIdentityInterface(const UObject* DerivedChild,
	                                            TScriptInterface<IAVVMOnlineIdentityInterface>& OutInterface);

	UFUNCTION(BlueprintCallable)
	static bool GetOuterOnlinePartyInterface(const UObject* DerivedChild,
	                                         TScriptInterface<IAVVMOnlinePartyInterface>& OutInterface);

	UFUNCTION(BlueprintCallable)
	static bool GetOuterOnlineChallengesInterface(const UObject* DerivedChild,
	                                              TScriptInterface<IAVVMOnlineChallengesInterface>& OutInterface);

	UFUNCTION(BlueprintCallable)
	static bool GetOuterOnlineBattlePassInterface(const UObject* DerivedChild,
	                                              TScriptInterface<IAVVMOnlineBattlePassInterface>& OutInterface);

	UFUNCTION(BlueprintCallable)
	static bool GetOuterOnlineStoreInterface(const UObject* DerivedChild,
	                                         TScriptInterface<IAVVMOnlineStoreInterface>& OutInterface);

	UFUNCTION(BlueprintCallable)
	static bool IsFirstPlayerHosting(const UObject* WorldContextObject,
	                                 const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface);

	static bool IsHosting(const FUniqueNetIdPtr PlayerUniqueNetIdPtr,
	                      const TScriptInterface<IAVVMOnlineIdentityInterface>& OnlineInterface);

	static FUniqueNetIdPtr GetUniqueNetIdPtr(const ULocalPlayer* Player);
	static ULocalPlayer* GetFirstLocalPlayer(const UObject* WorldContextObject);

private:
	template <typename UInterfaceClass, typename IInterfaceClass>
	static bool GetInterface(const UObject* DerivedChild,
	                         TScriptInterface<IInterfaceClass>& OutInterface);
};

template <typename UInterfaceClass, typename IInterfaceClass>
bool UAVVMOnlineInterfaceUtils::GetInterface(const UObject* DerivedChild,
                                             TScriptInterface<IInterfaceClass>& OutInterface)
{
	if (!IsValid(DerivedChild))
	{
		return false;
	}

	auto* Outer = Cast<UObject>(DerivedChild->GetImplementingOuterObject(UInterfaceClass::StaticClass()));
	if (!ensureAlways(IsValid(Outer)))
	{
		UE_LOG(LogUI,
		       Log,
		       TEXT("%s doesn't Implement %s"),
		       *DerivedChild->GetName(),
		       *UInterfaceClass::StaticClass()->GetName());
		return false;
	}

	OutInterface = TScriptInterface<IInterfaceClass>(Outer);

	const bool bImplement = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(OutInterface);
	if (!ensureAlways(bImplement))
	{
		UE_LOG(LogUI,
		       Log,
		       TEXT("%s is Null. Does %s implement the interface in Blueprint? If so, Update to support the interface Natively!"),
		       *UInterfaceClass::StaticClass()->GetName(),
		       *Outer->GetName());
	}

	return bImplement;
}
