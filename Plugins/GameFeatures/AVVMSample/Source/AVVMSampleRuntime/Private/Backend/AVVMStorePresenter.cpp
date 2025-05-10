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
#include "Backend/AVVMStorePresenter.h"

#include "AVVM.h"
#include "AVVMOnlineInterface.h"
#include "AVVMOnlineInterfaceUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "Backend/AVVMStoreViewModel.h"
#include "GameFramework/GameMode.h"

AActor* UAVVMStorePresenter::GetOuterKey() const
{
	return GetTypedOuter<AGameMode>();
}

void UAVVMStorePresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMStorePresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMStorePresenter::BP_OnNotificationReceived_ForcePullShopContent(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineStoreInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineStoreInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMStorePresenter::OnForcePullShopContentCompleted);

	UE_LOG(LogUI, Log, TEXT("Force Pull Store Content Request. In-Progress..."));
	OnlineInterface->ForcePullShopContent(Callback);
}

void UAVVMStorePresenter::BP_OnNotificationReceived_SellItem(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineStoreInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineStoreInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMStorePresenter::OnSellItemCompleted);

	const auto* Resource = Payload.GetPtr<FAVVMRuntimeResource>();
	if (!ensure(Resource != nullptr))
	{
		return;
	}

	UE_LOG(LogUI, Log, TEXT("Sell Item Request. In-Progress..."));
	OnlineInterface->SellItem(*Resource, Callback);
}

void UAVVMStorePresenter::BP_OnNotificationReceived_BuyItem(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	TScriptInterface<IAVVMOnlineStoreInterface> OnlineInterface;
	const bool bIsValid = UAVVMOnlineInterfaceUtils::GetOuterOnlineStoreInterface(this, OnlineInterface);
	if (!ensure(bIsValid))
	{
		return;
	}

	FAVVMOnlineResquestDelegate Callback;
	Callback.AddUObject(this, &UAVVMStorePresenter::OnBuyItemCompleted);

	const auto* Resource = Payload.GetPtr<FAVVMRuntimeResource>();
	if (!ensure(Resource != nullptr))
	{
		return;
	}

	UE_LOG(LogUI, Log, TEXT("Buy Item Request. In-Progress..."));
	OnlineInterface->BuyItem(*Resource, Callback);
}

void UAVVMStorePresenter::SetItems(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Updating local Items!"));

	auto* StoreViewModel = Cast<UAVVMStoreViewModel>(ViewModel.Get());
	if (IsValid(StoreViewModel))
	{
		StoreViewModel->SetStoreItems(FAVVMOnlineModule::GetJsonParser(), Payload);
	}
}

void UAVVMStorePresenter::StartPresenting()
{
	FAVVMPrimaryGameLayoutContextArgs ContextArgs;
	ContextArgs.LayerTag = TargetTag;
	ContextArgs.WidgetClass = WidgetClass;
	PushContentToPrimaryGameLayout(this, ContextArgs);
}

void UAVVMStorePresenter::StopPresenting()
{
	PopContentFromPrimaryGameLayout(this, ActivatableView.Get());
}

void UAVVMStorePresenter::BindViewModel() const
{
	const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
	UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, ActivatableView.Get());
}

void UAVVMStorePresenter::OnForcePullShopContentCompleted(const bool bWasSuccess,
                                                          const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Force Pull Store Content Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update store items onSuccess
		SetItems(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMStorePresenter::OnSellItemCompleted(const bool bWasSuccess,
                                              const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Sell Item Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update store items onSuccess
		SetItems(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}

void UAVVMStorePresenter::OnBuyItemCompleted(const bool bWasSuccess,
                                             const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	UE_LOG(LogUI, Log, TEXT("Buy Item Request Callback. Status: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));
	if (bWasSuccess)
	{
		// @gdemers update store items onSuccess
		SetItems(Payload);

		// @gdemers run any additional behaviour here!
		BP_OnRequestSuccess(Payload);
	}
	else
	{
		BP_OnRequestFailure(Payload);
	}
}
