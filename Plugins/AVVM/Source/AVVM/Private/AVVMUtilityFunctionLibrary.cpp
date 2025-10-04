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
#include "AVVMUtilityFunctionLibrary.h"

#include "AVVM.h"
#include "CommonUserWidget.h"
#include "MVVMSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "View/MVVMView.h"

void UAVVMUtilityFunctionLibrary::BindViewModel(const TScriptInterface<IAVVMViewModelFNameHelper>& ViewModelFNameHelper,
                                                UCommonUserWidget* Target)
{
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid<IAVVMViewModelFNameHelper>(ViewModelFNameHelper);
	if (!bIsValid)
	{
		return;
	}

	UMVVMView* MVVMView = UMVVMSubsystem::GetViewFromUserWidget(Target);
	if (IsValid(MVVMView))
	{
		const FName ViewModelFName = ViewModelFNameHelper->GetViewModelFName();
		UObject* ViewModel = ViewModelFNameHelper.GetObject();
		MVVMView->SetViewModel(ViewModelFName, ViewModel);
	}
}

ULocalPlayer* UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	ULocalPlayer* TargetLocalPlayer = UAVVMUtilityFunctionLibrary::GetTargetLocalPlayer(WorldContextObject);
	if (IsValid(TargetLocalPlayer))
	{
		return TargetLocalPlayer;
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

ULocalPlayer* UAVVMUtilityFunctionLibrary::GetTargetLocalPlayer(const UObject* WorldContextObject)
{
	auto* PlayerState = Cast<APlayerState>(WorldContextObject);
	if (IsValid(PlayerState))
	{
		return UAVVMUtilityFunctionLibrary::GetTargetLocalPlayer(PlayerState->GetPlayerController());
	}

	auto* PC = Cast<APlayerController>(WorldContextObject);
	if (IsValid(PC))
	{
		return PC->GetLocalPlayer();
	}

	return nullptr;
}
