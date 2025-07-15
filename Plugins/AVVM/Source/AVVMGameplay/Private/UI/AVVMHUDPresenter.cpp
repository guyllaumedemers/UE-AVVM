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
#include "UI/AVVMHUDPresenter.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "Cheats/AVVMTagRulesCheatData.h"
#include "GameFramework/GameStateBase.h"
#include "UI/AVVMHUDViewModel.h"

AActor* UAVVMHUDPresenter::GetOuterKey() const
{
	return GetTypedOuter<AGameStateBase>();
}

void UAVVMHUDPresenter::SafeBeginPlay()
{
	Super::SafeBeginPlay();
}

void UAVVMHUDPresenter::SafeEndPlay()
{
	Super::SafeEndPlay();
}

void UAVVMHUDPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* HUDViewModel = Cast<UAVVMHUDViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(HUDViewModel),
	                     TEXT("UAVVMHUDPresenter::ViewModel doesn't derive from UAVVMHUDViewModel!")))
	{
		HUDViewModel->SetPayload(Payload);
	}

	StartPresenting();
}

void UAVVMHUDPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMHUDPresenter::BP_OnNotificationReceived_ModifyVisibilityRequirements(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* HUDViewModel = Cast<UAVVMHUDViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(HUDViewModel),
	                     TEXT("UAVVMHUDPresenter::ViewModel doesn't derive from UAVVMHUDViewModel!")))
	{
		HUDViewModel->SetPayload(Payload);
	}
}

void UAVVMHUDPresenter::StartPresenting()
{
	FAVVMPrimaryGameLayoutContextArgs CtxArgs;
	CtxArgs.LayerTag = TargetTag;
	CtxArgs.WidgetClass = WidgetClass;
	IAVVMPrimaryGameLayoutInterface::PushContentToPrimaryGameLayout(this, CtxArgs);
}

void UAVVMHUDPresenter::StopPresenting()
{
	IAVVMPrimaryGameLayoutInterface::PopContentFromPrimaryGameLayout(this, ActivatableView.Get());
}

void UAVVMHUDPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}
