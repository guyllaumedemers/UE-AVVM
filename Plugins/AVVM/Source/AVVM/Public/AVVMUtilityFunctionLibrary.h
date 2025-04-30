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

#include "GameplayTags.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMUtilityFunctionLibrary.generated.h"

struct FAVVMNotificationPayload;
class IAVVMViewModelFNameHelper;
class UCommonUserWidget;

/**
 *	Class description:
 *
 *	UAVVMUtilityFunctionLibrary define reusable Api for general use-case.
 */
UCLASS()
class AVVM_API UAVVMUtilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template <typename T>
	static bool IsScriptInterfaceValid(const TScriptInterface<T>& Target);

	// @gdemers unreal doesnt allow fetching the current world from BP.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AVVM|Utility", meta=(WorldContext="Target"))
	static UWorld* BP_GetWorld(const UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AVVM|Utility", meta=(WorldContext="Target"))
	static FString GetMatchRequirement_ActorName(const UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AVVM|Utility", meta=(WorldContext="Target"))
	static FString GetMatchRequirement_ActorClassName(const UObject* Target);

	// @gdemers handle binding "Manual" ViewModel type to a Widget
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AVVM|Utility")
	static void BindViewModel(const TScriptInterface<IAVVMViewModelFNameHelper>& ViewModelFNameHelper,
	                          UCommonUserWidget* Target);

	// @gdemers utility function to be used when running comparison against tag channels
	static bool DoesExecute(const FGameplayTag& SrcTag,
	                        const FGameplayTag& ComparisonTag,
	                        const TInstancedStruct<FAVVMNotificationPayload>& Payload,
	                        TMulticastDelegate<void(const TInstancedStruct<FAVVMNotificationPayload>&)> Callback);
};

template <typename T>
bool UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(const TScriptInterface<T>& Target)
{
	return (Target.GetInterface() != nullptr && IsValid(Target.GetObject()));
}
