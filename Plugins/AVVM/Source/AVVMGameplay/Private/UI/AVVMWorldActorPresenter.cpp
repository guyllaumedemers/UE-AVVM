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
#include "UI/AVVMWorldActorPresenter.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonUserWidget.h"
#include "MVVMViewModelBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"

AActor* UAVVMWorldActorPresenter::GetOuterKey() const
{
	return GetTypedOuter<AActor>();
}

void UAVVMWorldActorPresenter::SafeBeginPlay()
{
	Super::SafeBeginPlay();

	const bool bDoesDisplayInWorld = (PreviewType == EAVVMWidgetPreviewType::InWorld);
	if (bDoesDisplayInWorld)
	{
		SetupWorldWidget();
	}
}

void UAVVMWorldActorPresenter::SafeEndPlay()
{
	Super::SafeEndPlay();

	OwningOuter.Reset();
	WorldWidget.Reset();
}

void UAVVMWorldActorPresenter::BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StartPresenting();
}

void UAVVMWorldActorPresenter::BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload)
{
	StopPresenting();
}

void UAVVMWorldActorPresenter::StartPresenting()
{
	const bool bDoesDisplayOnHUD = (PreviewType == EAVVMWidgetPreviewType::OnHUD);
	if (bDoesDisplayOnHUD)
	{
		FAVVMUIExtensionContextArgs CtxArgs;
		CtxArgs.World = GetWorld();
		CtxArgs.ContextObject = this;
		CtxArgs.ViewModel = ViewModel.Get();
		CtxArgs.ExtensionPointTag = ExtensionPointTag;

		ExtensionRequestHandle = IAVVMUIExtensionInterface::PushContentToExtensionPoint(CtxArgs);
		return;
	}

	UCommonUserWidget* TargetWidget = WorldWidget.Get();
	if (IsValid(TargetWidget))
	{
		TargetWidget->SetVisibility(DefaultVisibilityOnStart);
	}
}

void UAVVMWorldActorPresenter::StopPresenting()
{
	const bool bDoesDisplayOnHUD = (PreviewType == EAVVMWidgetPreviewType::OnHUD);
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

void UAVVMWorldActorPresenter::SetupWorldWidget()
{
	const AActor* Outer = GetTypedOuter<AActor>();
	if (!IsValid(Outer))
	{
		return;
	}

	auto* WidgetComponent = Outer->GetComponentByClass<UWidgetComponent>();
	if (!IsValid(WidgetComponent))
	{
		UE_LOG(LogUI,
		       Log,
		       TEXT("Actor \"%s\" doesn't hold a WidgetComponent. Failed to set \"%s\" Widget Class."),
		       *Outer->GetName(),
		       IsValid(PreviewInteractionWidgetClass) ? *PreviewInteractionWidgetClass->GetName() : TEXT("Unknown"));

		return;
	}

	WidgetComponent->SetWidgetClass(PreviewInteractionWidgetClass);

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
