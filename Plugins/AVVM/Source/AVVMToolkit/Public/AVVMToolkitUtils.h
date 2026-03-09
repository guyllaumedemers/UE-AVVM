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

#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMToolkitUtils.generated.h"

class IAVVMViewModelFNameHelper;
class UCommonUserWidget;

/**
 *	Class description:
 *
 *	UAVVMToolkitUtils define reusable Api for general use-case.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMToolkitUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template <typename T>
	static bool IsNativeScriptInterfaceValid(const TScriptInterface<T>& Target);

	template <typename U>
	static bool IsBlueprintScriptInterfaceValid(const UObject* Target);

	UFUNCTION(BlueprintCallable, Category="AVVMToolkit|Utils", meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static ULocalPlayer* GetFirstOrTargetLocalPlayer(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="AVVMToolkit|Utils", meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static ULocalPlayer* GetTargetLocalPlayer(const UObject* WorldContextObject);

	// @gdemers handle binding "Manual" ViewModel type to a Widget
	UFUNCTION(BlueprintCallable, Category="AVVMToolkit|Utils")
	static void BindViewModel(const TScriptInterface<IAVVMViewModelFNameHelper>& ViewModelFNameHelper,
							  UCommonUserWidget* Target);
};

template <typename T>
bool UAVVMToolkitUtils::IsNativeScriptInterfaceValid(const TScriptInterface<T>& Target)
{
	const bool bIsValidInterfaceObject = (Target.GetInterface() != nullptr && IsValid(Target.GetObject()));
	return ensureAlwaysMsgf(bIsValidInterfaceObject,
	                        TEXT("%hs line:%d."),
	                        __FUNCTION__,
	                        __LINE__);
}

template <typename U>
bool UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid(const UObject* Target)
{
	const bool bDoesBPImplementInterface = IsValid(Target) ? Target->GetClass()->ImplementsInterface(U::StaticClass()) : false;
	return ensureAlwaysMsgf(bDoesBPImplementInterface,
	                        TEXT("%hs line:%d."),
	                        __FUNCTION__,
	                        __LINE__);
}
