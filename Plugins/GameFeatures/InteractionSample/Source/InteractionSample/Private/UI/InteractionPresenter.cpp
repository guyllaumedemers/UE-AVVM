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
#include "UI/InteractionPresenter.h"

#include "AVVM.h"
#include "AVVMGameStateHandshakeComponent.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonUserWidget.h"
#include "MVVMViewModelBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Data/AVVMHearbeatPayload.h"
#include "Engine/AssetManager.h"
#include "UI/InteractionViewModel.h"

AActor* UInteractionPresenter::GetOuterKey() const
{
	return GetTypedOuter<AActor>();
}

void UInteractionPresenter::SafeBeginPlay(const UWorld* World)
{
	Super::SafeBeginPlay(World);

	const bool bDoesDisplayInWorld = (PreviewType == EWidgetPreviewType::InWorld);
	if (bDoesDisplayInWorld)
	{
		DeferredWidgetClassLoading();
	}
}

void UInteractionPresenter::SafeEndPlay()
{
	Super::SafeEndPlay();

	OwningOuter.Reset();
	WorldWidget.Reset();
}

void UInteractionPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* InteractionViewModel = Cast<UInteractionViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(InteractionViewModel),
	                     TEXT("UInteractionPresenter::ViewModel doesn't derive from UInteractionViewModel!")))
	{
		InteractionViewModel->SetPayload(Payload);
	}

	StartPresenting();
}

void UInteractionPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UInteractionPresenter::BP_OnNotificationReceived_PumpHeartbeat(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* InteractionViewModel = Cast<UInteractionViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(InteractionViewModel),
	                     TEXT("UInteractionPresenter::ViewModel doesn't derive from UInteractionViewModel!")))
	{
		const auto* NewHeartbeat = Payload.GetPtr<FAVVMHearbeatPayload>();
		InteractionViewModel->PumpHeartbeat((NewHeartbeat != nullptr) ? NewHeartbeat->Value : static_cast<float>(INDEX_NONE));
	}
}

void UInteractionPresenter::BP_OnNotificationReceived_Kill(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	auto* InteractionViewModel = Cast<UInteractionViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(InteractionViewModel),
	                     TEXT("UInteractionPresenter::ViewModel doesn't derive from UInteractionViewModel!")))
	{
		InteractionViewModel->Kill();
	}
}

void UInteractionPresenter::BP_OnNotificationReceived_Execute(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	const auto* HandshakeComponent = UAVVMGameStateHandshakeComponent::GetActorComponent(this);
	if (ensureAlwaysMsgf(IsValid(HandshakeComponent),
	                     TEXT("Missing Handshake component on AGameStateBase!")))
	{
		FOnHandshakeRequestComplete Callback;
		Callback.BindDynamic(this, &UInteractionPresenter::PostHandshakeValidation);
		HandshakeComponent->ProcessHandshake(Payload, Callback);
	}
}

void UInteractionPresenter::StartPresenting()
{
	const bool bDoesDisplayOnHUD = (PreviewType == EWidgetPreviewType::OnHUD);
	if (bDoesDisplayOnHUD)
	{
		FAVVMUIExtensionContextArgs CtxArgs;
		CtxArgs.World = GetWorld();
		CtxArgs.ContextObject = this;
		CtxArgs.ViewModel = ViewModel.Get();
		CtxArgs.ExtensionPointTag = TargetTag;

		ExtensionRequestHandle = IAVVMUIExtensionInterface::PushContentToExtensionPoint(CtxArgs);
		return;
	}

	UCommonUserWidget* TargetWidget = WorldWidget.Get();
	if (IsValid(TargetWidget))
	{
		TargetWidget->SetVisibility(DefaultVisibilityOnStart);
	}
}

void UInteractionPresenter::StopPresenting()
{
	const bool bDoesDisplayOnHUD = (PreviewType == EWidgetPreviewType::OnHUD);
	if (bDoesDisplayOnHUD)
	{
		IAVVMUIExtensionInterface::PopContentToExtensionPoint(ExtensionRequestHandle);
		return;
	}

	UCommonUserWidget* TargetWidget = WorldWidget.Get();
	if (IsValid(TargetWidget))
	{
		TargetWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInteractionPresenter::DeferredWidgetClassLoading()
{
	if (!ensureAlwaysMsgf(!WorldWidgetClass.IsNull(),
	                      TEXT("Cannot load World Widget Class. Is Null!")))
	{
		return;
	}

	if (!WorldWidgetClass.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UInteractionPresenter::SetupWorldWidget);
		UAssetManager::Get().LoadAssetList({WorldWidgetClass.ToSoftObjectPath()}, Callback);
	}
	else
	{
		SetupWorldWidget();
	}
}

void UInteractionPresenter::SetupWorldWidget()
{
	const AActor* Outer = GetTypedOuter<AActor>();
	if (!IsValid(Outer))
	{
		return;
	}

	auto* WidgetComponent = Outer->GetComponentByClass<UWidgetComponent>();
	if (!ensureAlwaysMsgf(IsValid(WidgetComponent),
	                      TEXT("Missing UWidgetComponent on Actor \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	WidgetComponent->SetWidgetClass(WorldWidgetClass.Get());

	auto* Target = Cast<UCommonUserWidget>(WidgetComponent->GetWidget());
	if (IsValid(Target))
	{
		DefaultVisibilityOnStart = Target->GetVisibility();
		WorldWidget = Target;
		Target->SetVisibility(ESlateVisibility::Collapsed);

		const auto ViewModelFNameHelper = TScriptInterface<IAVVMViewModelFNameHelper>(ViewModel.Get());
		UAVVMUtilityFunctionLibrary::BindViewModel(ViewModelFNameHelper, Target);
	}

	OwningOuter = Outer;
}

void UInteractionPresenter::PostHandshakeValidation(const bool bWasSuccess,
                                                    const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	if (!bWasSuccess)
	{
		return;
	}

	auto* InteractionViewModel = Cast<UInteractionViewModel>(ViewModel.Get());
	if (ensureAlwaysMsgf(IsValid(InteractionViewModel),
	                     TEXT("UInteractionPresenter::ViewModel doesn't derive from UInteractionViewModel!")))
	{
		InteractionViewModel->Execute();
	}

	UE_AVVM_NOTIFY(this,
	               PostInteractionChannelTag,
	               OwningOuter.Get(),
	               Payload);
}
