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
#include "UIExtensionSystem.h"
#include "UObject/Interface.h"

#include "AVVMUIExtensionInterface.generated.h"

/**
 *	Class description:
 *
 *	FAVVMUIExtensionContextArgs encapsulate arguments to pass to the UI Extension Point.
 */
USTRUCT(BlueprintType)
struct AVVMSAMPLERUNTIME_API FAVVMUIExtensionContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	UWorld* World = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	UObject* ContextObject = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite, meta=(MustImplement="NotifyFieldValueChanged"))
	UObject* ViewModel = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag ExtensionPointTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	UAVVMUIExtensionInterface define reusable api to be implemented by presenter class to handle
 *	recurrent actions like Start/Stop presenting which interface with Unreal Extension Point system.
 *
 *	Note : It's expected that your Presenter class only implement one of the following interface :
 *	IAVVMUIExtensionInterface or IAVVMPrimaryGameLayoutInterface.
 */
UINTERFACE(BlueprintType, Blueprintable)
class UAVVMUIExtensionInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMSAMPLERUNTIME_API IAVVMUIExtensionInterface
{
	GENERATED_BODY()

protected:
	// @gdemers please implement FOnConfigureWidgetForData ConfigureWidgetForData; on the target ExtensionPointWidget
	// in order to act on the received ExtensionRequest which will contain the ViewModel
	// as Data and allow VM binding to be setup via UAVVMUtilityFunctionLibrary::BindViewModel.
	FUIExtensionHandle PushContentToExtensionPoint(const FAVVMUIExtensionContextArgs& ContextArgs);
	void PopContentToExtensionPoint(FUIExtensionHandle& ExtensionHandle);
};
