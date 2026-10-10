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
#include "InventoryToolkitWindow.h"

#include "AVVMLogger.h"
#include "InventoryProviderDataEditObject.h"
#include "InventorySettings.h"
#include "ItemDataEditObject.h"
#include "Android/AndroidPlatformApplicationMisc.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "SWidgets/AVVMEditorToolkitDataEditor.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWindow.h"

void SInventoryToolkitWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
				.Text(this, &SInventoryToolkitWindow::OnPresentPlugin)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SAVVMEditorToolkitDataImporter)
				.DataRegistryTypes(GetInventoryDataRegistryTypes())
				.OnDataImporterSourceChanged(this, &SInventoryToolkitWindow::OnDataImporterSourceChanged)
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 2.0f))
			.FillContentHeight(1.f)
			[
				SAssignNew(DataVisualizer, SAVVMEditorToolkitDataVisualizer)
				.Visibility(this, &SInventoryToolkitWindow::OnDataVisualizerVisibilityStateChanged)
				.OnButtonClick_Create(this, &SInventoryToolkitWindow::OnButtonClick_Create)
				.OnButtonClick_Edit(this, &SInventoryToolkitWindow::OnButtonClick_Edit)
				.OnButtonClick_Delete(this, &SInventoryToolkitWindow::OnButtonClick_Delete)
			]
		]
	];
}

FText SInventoryToolkitWindow::OnPresentPlugin() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "Plugin Presentation", "Welcome! lorem ipsum...");
}

const UAVVMEditorToolkitDataEditObject* SInventoryToolkitWindow::GetDataEditObject(const FName RegistryType) const
{
	const TArray<FName> RegistryTypes{GetInventoryDataRegistryTypes()};

	const bool bDoesContains = RegistryTypes.Contains(RegistryType);
	if (!bDoesContains)
	{
		return nullptr;
	}

	const auto GetClassCDO = []<typename TClass>()
	{
		const UClass* TargetClass = TClass::StaticClass();
		return IsValid(TargetClass) ? TargetClass->GetDefaultObject<TClass>() : nullptr;
	};

	if (RegistryType.IsEqual(UInventorySettings::GetItemGroupRegistryType()))
	{
		return GetClassCDO.operator()<UItemGroupDataEditObject>();
	}
	else if (RegistryType.IsEqual(UInventorySettings::GetItemRegistryType()))
	{
		return GetClassCDO.operator()<UItemDataEditObject>();
	}
	else if (RegistryType.IsEqual(UInventorySettings::GetFtueInventoryProviderRegistryType()))
	{
		return GetClassCDO.operator()<UFtueInventoryProviderDataEditObject>();
	}
	else if (RegistryType.IsEqual(UInventorySettings::GetStubDataInventoryDependencyGraphRegistryType()))
	{
		return GetClassCDO.operator()<UStubDataInventoryDependencyGraphEditObject>();
	}

	return nullptr;
}

bool SInventoryToolkitWindow::OnDataImporterSourceChanged(FName SelectedSourceType)
{
	SelectedDataRegistryType = FDataRegistryType{SelectedSourceType};
	if (DataVisualizer.IsValid())
	{
		DataVisualizer->UpdateDataVisualizer(SelectedDataRegistryType);
	}

	return SelectedDataRegistryType.IsValid();
}

EVisibility SInventoryToolkitWindow::OnDataVisualizerVisibilityStateChanged() const
{
	return SelectedDataRegistryType.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

TArray<FName> SInventoryToolkitWindow::GetInventoryDataRegistryTypes() const
{
	return {
			UInventorySettings::GetItemGroupRegistryType(),
			UInventorySettings::GetItemRegistryType(),
			UInventorySettings::GetFtueInventoryProviderRegistryType(),
			UInventorySettings::GetStubDataInventoryDependencyGraphRegistryType()
	};
}

FReply SInventoryToolkitWindow::OnButtonClick_Create()
{
	if (!SelectedDataRegistryType.IsValid())
	{
		return FReply::Unhandled();
	}
	else
	{
		OpenCreateWindow(SelectedDataRegistryType);
		return FReply::Handled();
	}
}

FReply SInventoryToolkitWindow::OnButtonClick_Edit()
{
	if (!SelectedDataRegistryType.IsValid() || !DataVisualizer.IsValid())
	{
		return FReply::Unhandled();
	}

	bool bResult{false};

	TArray<FName> OutItemNames{};
	bResult = DataVisualizer->GetSelectedItems(OutItemNames);

	for (const auto& ItemName : OutItemNames)
	{
		OpenEditWindow(SelectedDataRegistryType, ItemName);
	}

	return bResult ? FReply::Handled() : FReply::Unhandled();
}

FReply SInventoryToolkitWindow::OnButtonClick_Delete()
{
	if (SelectedDataRegistryType.IsValid())
	{
		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

void SInventoryToolkitWindow::OpenCreateWindow(const FName RegistryType)
{
	AVVM_EDITOR_LOGGER_LOG(TEXT("Create new entry of RegistryType %s."),
	                       *RegistryType.ToString());

	FDisplayMetrics DisplayMetrics{};
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayMetrics.PrimaryDisplayWorkAreaRect.Left, DisplayMetrics.PrimaryDisplayWorkAreaRect.Top);

	const FVector2D ClientSize(1200.0f * DPIScaleFactor, 800.0f * DPIScaleFactor);

	auto NewFloatingWindow = SNew(SWindow)
		.Title(NSLOCTEXT("AVVMEditorToolkit", "WindowTitle", "Floating Window"))
		.CreateTitleBar(true)
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		.IsInitiallyMaximized(false)
		.IsInitiallyMinimized(false)
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(ClientSize)
		.AdjustInitialSizeAndPositionForDPIScale(false)
		.Content()
		[
			SNew(SAVVMEditorToolkitDataEditor)
			.DataEditObject(GetDataEditObject(RegistryType))
			.SelectedDataRegistryType(FDataRegistryType{RegistryType})
		];

	NewFloatingWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosedOverride));
	NewFloatingWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosed));

	FloatingWindows.Add(NewFloatingWindow);

	FSlateApplication::Get().AddWindow(NewFloatingWindow, true);
	FGlobalTabmanager::Get()->SetRootWindow(NewFloatingWindow);
	FGlobalTabmanager::Get()->SetAllowWindowMenuBar(true);
	FSlateNotificationManager::Get().SetRootWindow(NewFloatingWindow);
}

void SInventoryToolkitWindow::OpenEditWindow(const FName RegistryType, const FName ItemName)
{
	// TODO @gdemers make a context window for editing element
	AVVM_EDITOR_LOGGER_LOG(TEXT("Edit entry of RegistryType: %s, ItemName: %s."),
	                       *RegistryType.ToString(),
	                       *ItemName.ToString());
}

void SInventoryToolkitWindow::OnWindowClosedOverride(const TSharedRef<SWindow>& PendingCloseWindow) const
{
	const auto& CtxWindow = static_cast<const SAVVMEditorToolkitDataEditor&>(PendingCloseWindow->GetContent().Get());
	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, CtxWindow.GetModalMessage_OnClosure(), CtxWindow.GetModalTitle_OnClosure());
	if (Response == EAppReturnType::Yes)
	{
		PendingCloseWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride());
		PendingCloseWindow->RequestDestroyWindow();
	}
}

void SInventoryToolkitWindow::OnWindowClosed(const TSharedRef<SWindow>& PendingCloseWindow)
{
	if (ensureAlwaysMsgf(FloatingWindows.Contains(PendingCloseWindow), TEXT("Attempt to remove Window multiple times.")))
	{
		FloatingWindows.Remove(PendingCloseWindow);
	}
}
