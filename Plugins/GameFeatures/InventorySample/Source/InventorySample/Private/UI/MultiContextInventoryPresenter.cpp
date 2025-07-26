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
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "Ability/AVVMAbilityUtils.h"
#include "UI/MultiContextInventoryViewModel.h"

AActor* UMultiContextInventoryPresenter::GetOuterKey() const
{
	return GetTypedOuter<AActor>();
}

void UMultiContextInventoryPresenter::SafeEndPlay()
{
	Super::SafeEndPlay();
	Target.Reset();
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

	const auto* Handshake = Payload.GetPtr<FAVVMHandshakePayload>();
	if (Handshake != nullptr)
	{
		// @gdemers Expect to receive a PlayerState here (due to ASC ownership). Always! If interaction with a world object, the PC with net authority
		// can execute Ability such as PlayerToggleInventory, and any PlayerInteractionAbility with either Role_Authority or Role_AutonomousProxy resulting in triggering of local ui.
		const AActor* PlayerState = Handshake->Target.Get();
		OwnerASC = UAVVMAbilityUtils::GetAbilitySystemComponent(PlayerState);
		Target = PlayerState;
	}

	StartPresenting();
}

void UMultiContextInventoryPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UMultiContextInventoryPresenter::StartPresenting()
{
	ULocalPlayer* LocalPlayer = UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(Target.Get());
	if (!ensureAlwaysMsgf(IsValid(LocalPlayer),
	                      TEXT("UMultiContextInventoryPresenter couldn't find a valid LocalPlayer!")))
	{
		return;
	}

	auto* AbilityComponent = OwnerASC.Get();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AddReplicatedLooseGameplayTags(GrantAbilityTags);
	}

	FAVVMPrimaryGameLayoutContextArgs CtxArgs;
	CtxArgs.LayerTag = TargetTag;
	CtxArgs.WidgetClass = WidgetClass;
	IAVVMPrimaryGameLayoutInterface::PushContentToPrimaryGameLayout(this, LocalPlayer, CtxArgs);
}

void UMultiContextInventoryPresenter::StopPresenting()
{
	auto* AbilityComponent = OwnerASC.Get();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveReplicatedLooseGameplayTags(GrantAbilityTags);
		OwnerASC.Reset();
	}

	ULocalPlayer* LocalPlayer = UAVVMUtilityFunctionLibrary::GetFirstOrTargetLocalPlayer(Target.Get());
	if (IsValid(LocalPlayer))
	{
		IAVVMPrimaryGameLayoutInterface::PopContentFromPrimaryGameLayout(LocalPlayer, ActivatableView.Get());
		Target.Reset();
	}
}

void UMultiContextInventoryPresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}
