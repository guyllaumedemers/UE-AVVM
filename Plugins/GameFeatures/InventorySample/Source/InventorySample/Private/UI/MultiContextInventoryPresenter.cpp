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
#include "UI/MultiContextInventoryPresenter.h"

#include "AbilitySystemComponent.h"
#include "AVVM.h"
#include "AVVMGameplayUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "MVVMViewModelBase.h"
#include "GameFramework/PlayerState.h"
#include "UI/MultiContextInventoryViewModel.h"

AActor* UMultiContextInventoryPresenter::GetOuterKey() const
{
	return GetTypedOuter<AActor>();
}

void UMultiContextInventoryPresenter::SafeBeginPlay()
{
	Super::SafeBeginPlay();

	// @gdemers only true if held by the PlayerState, other Actor types like a Shop, a mystery box, etc...
	// will early out and only initialize once they receive a context payload with information about the interaction end point.
	const auto* PlayerState = GetTypedOuter<APlayerState>();
	if (!IsValid(PlayerState))
	{
		return;
	}

	APlayerController* PlayerController = PlayerState->GetPlayerController();
	if (!IsValid(PlayerController))
	{
		return;
	}

	const bool bHasActorAuthority = UAVVMGameplayUtils::CheckActorAuthority(PlayerController);
	if (bHasActorAuthority)
	{
		OwnerASC = PlayerState->GetComponentByClass<UAbilitySystemComponent>();
	}
}

void UMultiContextInventoryPresenter::SafeEndPlay()
{
	Super::SafeEndPlay();
	OwnerASC.Reset();
}

void UMultiContextInventoryPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* MultiContextInventoryViewModel = Cast<UMultiContextInventoryViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(MultiContextInventoryViewModel),
	                     TEXT("UMultiContextInventoryPresenter::ViewModel doesn't derive from UMultiContextInventoryViewModel!")))
	{
		MultiContextInventoryViewModel->SetPayload(Payload);
	}

	StartPresenting();
}

void UMultiContextInventoryPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UMultiContextInventoryPresenter::StartPresenting()
{
	FAVVMPrimaryGameLayoutContextArgs CtxArgs;
	CtxArgs.LayerTag = TargetTag;
	CtxArgs.WidgetClass = WidgetClass;
	IAVVMPrimaryGameLayoutInterface::PushContentToPrimaryGameLayout(this, CtxArgs);

	auto* AbilityComponent = OwnerASC.Get();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AddReplicatedLooseGameplayTags(GrantAbilityTags);
	}
}

void UMultiContextInventoryPresenter::StopPresenting()
{
	IAVVMPrimaryGameLayoutInterface::PopContentFromPrimaryGameLayout(this, ActivatableView.Get());

	auto* AbilityComponent = OwnerASC.Get();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveReplicatedLooseGameplayTags(GrantAbilityTags);
	}
}

void UMultiContextInventoryPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}
