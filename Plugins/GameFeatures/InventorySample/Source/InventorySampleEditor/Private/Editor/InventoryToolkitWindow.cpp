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

#include "AssetToolsModule.h"
#include "AVVMEditorToolkitModule.h"
#include "AVVMEditorToolkitUtils.h"
#include "AVVMLogger.h"
#include "InventoryProviderDataEditObject.h"
#include "InventorySettings.h"
#include "ItemDataEditObject.h"
#include "SCheckBoxList.h"
#include "Data/InventoryProviderTableRow.h"
#include "Data/InventoryStubDataProviderTableRow.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Factories/DataTableFactory.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/MessageDialog.h"
#include "SWidgets/AVVMEditorToolkitDataEditor.h"
#include "SWidgets/AVVMEditorToolkitDataTablePicker.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"
#include "SWidgets/AVVMEditorToolkitPluginHeader.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/SRichTextBlock.h"

void SInventoryToolkitWindow::Construct(const FArguments& InArgs)
{
	SAssignNew(CheckBoxList, SCheckBoxList)
	.IncludeGlobalCheckBoxInHeaderRow(false)
	.OnItemCheckStateChanged(this, &SInventoryToolkitWindow::OnCheckBoxStateChanged)
	.ToolTipText(NSLOCTEXT("AVVMEditorToolkit", "ChecklistTooltip", "During the creation process, dependent assets will be created for the type selected. Unless unchecked!"))
	.ItemHeaderLabel(NSLOCTEXT("AVVMEditorToolkit", "ChecklistHeader", "Generate Dependent Assets"));

	if (CheckBoxList.IsValid())
	{
		for (const auto& Element : GetAssetTypeToDefaultGenerate())
		{
			CheckBoxList->AddItem(FText::FromName(Element), true);
		}
		
		bCheckBoxStates = CheckBoxList->GetValues();
	}
	
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
				SNew(SAVVMEditorToolkitPluginHeader)
				.Title(this, &SInventoryToolkitWindow::PluginTitle)
				.Description(this, &SInventoryToolkitWindow::PluginDescription)
				.Version(this, &SInventoryToolkitWindow::PluginVersion)
				.Icon(this, &SInventoryToolkitWindow::PluginIcon)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				CheckBoxList.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SAVVMEditorToolkitOutputPathPicker)
				.OnPathSelected(this, &SInventoryToolkitWindow::OnOutputAssetPathSelected)
				.AssetPathHeaderText(NSLOCTEXT("AVVMEditorToolkit", "AssetPathHeader", "Select an Output Path:"))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SAVVMEditorToolkitDataImporter)
				.Elements(GetInventoryDataRegistryTypes())
				.Visibility(this, &SInventoryToolkitWindow::OnVisibility_AssetPathSelectionChanged)
				.OnDataImporterSourceChanged(this, &SInventoryToolkitWindow::OnDataRegistrySelectionChanged)
				.DropDownHeaderText(NSLOCTEXT("AVVMEditorToolkit", "DataImporterHeader", "Select a DataRegistry Source:"))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.FillHeight(1.f)
			[
				SAssignNew(DataTablePicker, SAVVMEditorToolkitDataTablePicker)
				.RegistryType(this, &SInventoryToolkitWindow::GetDataRegistryType)
				.Visibility(this, &SInventoryToolkitWindow::OnVisibility_DataTableAvailableChanged)
				.OnSelectionChanged(this, &SInventoryToolkitWindow::OnDataTableSelectionChanged)
				.PickerHeaderText(NSLOCTEXT("AVVMEditorToolkit", "PickerHeader", "Select a Data Table Source:"))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.ButtonColorAndOpacity(FLinearColor::Green)
				.Visibility(this, &SInventoryToolkitWindow::OnVisibility_DataRegistrySelectionChanged)
				.OnClicked(this, &SInventoryToolkitWindow::OnButtonClick_OpenWindow)
				.Text(this, &SInventoryToolkitWindow::OnTextChanged_CreateOrEdit)
			]
		]
	];
}

FText SInventoryToolkitWindow::PluginTitle() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "SInventoryToolkitWindow::PluginDescription", "Inventory Editor");
}

FText SInventoryToolkitWindow::PluginDescription() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "SInventoryToolkitWindow::PluginDescription", "A toolkit for generating data-driven items for gameplay.");
}

FText SInventoryToolkitWindow::PluginVersion() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "SInventoryToolkitWindow::PluginDescription", "v0.0.1");
}

FSlateBrush* SInventoryToolkitWindow::PluginIcon() const
{
	return nullptr;
}

bool SInventoryToolkitWindow::OnDataRegistrySelectionChanged(const FName SelectedRegistryType,
                                                             const int32 NewSelectedIndex)
{
	SelectedDataRegistryType = FDataRegistryType{SelectedRegistryType};
	SelectedRegistryTypeIndex = NewSelectedIndex;

	bool bResult{false};
	if (!ensureAlwaysMsgf(SelectedDataRegistryType.IsValid(), TEXT("Invalid RegistryType.")))
	{
		return bResult;
	}

	SelectedDataRegistry = UAVVMEditorToolkitUtils::GetDataRegistry(SelectedDataRegistryType);
	if (!SelectedDataRegistry.IsValid() && (bCheckBoxStates.IsValidIndex(1/*UDataRegistry*/) && bCheckBoxStates[1]))
	{
		bResult = CreateNewDataRegistry(SelectedRegistryTypeIndex);
	}
	else if (SelectedDataRegistry.IsValid() && DataTablePicker.IsValid())
	{
		bResult = bHasDataTableAvailable = DataTablePicker->UpdateDataTableCollection(SelectedDataRegistryType);
	}
	else
	{
		FMessageDialog::Open(EAppMsgCategory::Error,
		                     EAppMsgType::Ok,
		                     NSLOCTEXT("AVVMEditorToolkit", "NotifyDataRegistryMissing", "Failed to retrieve valid Data Registry."));
	}

	return bResult;
}

void SInventoryToolkitWindow::OnCheckBoxStateChanged(int32 Index)
{
	if (bCheckBoxStates.IsValidIndex(Index))
	{
		bool& Value = bCheckBoxStates[Index];
		Value = !Value;
	}
}

void SInventoryToolkitWindow::OnOutputAssetPathSelected(const FString& NewPath)
{
	SelectedAssetPath = NewPath;
}

void SInventoryToolkitWindow::OnDataTableSelectionChanged(TWeakObjectPtr<UDataTable> NewSelection,
														  ESelectInfo::Type SelectInfo)
{
	bShouldEditDataTable = NewSelection.IsValid();
	if (bShouldEditDataTable)
	{
		SelectedDataTable = NewSelection;
	}
}

EVisibility SInventoryToolkitWindow::OnVisibility_AssetPathSelectionChanged() const
{
	return SelectedAssetPath.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SInventoryToolkitWindow::OnVisibility_DataTableAvailableChanged() const
{
	return (SelectedDataRegistryType.IsValid() && bHasDataTableAvailable) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SInventoryToolkitWindow::OnVisibility_DataRegistrySelectionChanged() const
{
	return SelectedDataRegistryType.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

TArray<FName> SInventoryToolkitWindow::GetAssetTypeToDefaultGenerate() const
{
	return {
			GetFNameSafe(UDataRegistry::StaticClass()),
			GetFNameSafe(UDataTable::StaticClass()),
			GetFNameSafe(UDataAsset::StaticClass()),
			GetFNameSafe(UBlueprint::StaticClass())
	};
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

TArray<UScriptStruct*> SInventoryToolkitWindow::GetInventoryRowTypes() const
{
	return {
			FItemGroupDefinitionDataTableRow::StaticStruct(),
			FItemDefinitionDataTableRow::StaticStruct(),
			FFtue_InventoryProviderTableRow::StaticStruct(),
			FStubData_InventoryStubDataProviderTableRow::StaticStruct()
	};
}

FDataRegistryType SInventoryToolkitWindow::GetDataRegistryType() const
{
	return SelectedDataRegistryType;
}

FReply SInventoryToolkitWindow::OnButtonClick_OpenWindow()
{
	if (!SelectedDataRegistryType.IsValid())
	{
		return FReply::Unhandled();
	}

	if (bShouldEditDataTable)
	{
		EditDataTable();
	}
	else
	{
		CreateDataTable();
	}

	return FReply::Handled();
}

FText SInventoryToolkitWindow::OnTextChanged_CreateOrEdit() const
{
	return (bShouldEditDataTable && bHasDataTableAvailable) ? NSLOCTEXT("AVVMEditorToolkit", "DataTable_Edit", "Edit") : NSLOCTEXT("AVVMEditorToolkit", "DataTable_Create", "Create");
}

void SInventoryToolkitWindow::CreateDataTable()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return;
	}

	auto* Factory = NewObject<UDataTableFactory>();
	if (IsValid(Factory))
	{
		const auto RowTypes = GetInventoryRowTypes();
		Factory->Struct = RowTypes.IsValidIndex(SelectedRegistryTypeIndex) ? RowTypes[SelectedRegistryTypeIndex] : FAVVMDataTableRow::StaticStruct();
	}

	auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	SelectedDataTable = Cast<UDataTable>(AssetTools.Get().CreateAsset(FString::Printf(TEXT("DT_%s"), PROJECT_ASSET_PREFIX),
	                                                                  SelectedAssetPath,
	                                                                  UDataTable::StaticClass(),
	                                                                  Factory));

	if (ensureAlwaysMsgf(SelectedDataTable.IsValid(), TEXT("Failed to create new Data Table.")))
	{
		OpenWindow(SelectedDataRegistryType, bShouldEditDataTable);
	}
}

void SInventoryToolkitWindow::EditDataTable()
{
}

void SInventoryToolkitWindow::OpenWindow(const FName RegistryType,
                                         const bool bShouldOpenInEditMode)
{
	AVVM_EDITOR_LOGGER_LOG(TEXT("Create new entry of RegistryType %s."),
	                       *RegistryType.ToString());

	FDisplayMetrics DisplayMetrics{};
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayMetrics.PrimaryDisplayWorkAreaRect.Left, DisplayMetrics.PrimaryDisplayWorkAreaRect.Top);

	const FVector2D ClientSize(1200.0f * DPIScaleFactor, 800.0f * DPIScaleFactor);

	auto NewFloatingWindow = SNew(SWindow)
		.Title(NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_Create", "SInventoryToolkitWindow::Factory"))
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
			.SelectedDataRegistryType(FDataRegistryType{RegistryType})
			.SelectedItemName(NAME_None)
			.DataEditorObject(GetDataEditObject(RegistryType))
			.OutputDataTable(SelectedDataTable)
			.Mode(bShouldOpenInEditMode ? EAVVMDataEditorModes::Edit : EAVVMDataEditorModes::Create)
		];

	NewFloatingWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosedOverride));
	NewFloatingWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosed));

	FloatingWindows.Add(NewFloatingWindow);

	FSlateApplication::Get().AddWindow(NewFloatingWindow, true);
	FGlobalTabmanager::Get()->SetRootWindow(NewFloatingWindow);
	FGlobalTabmanager::Get()->SetAllowWindowMenuBar(true);
	FSlateNotificationManager::Get().SetRootWindow(NewFloatingWindow);
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

bool SInventoryToolkitWindow::CreateNewDataRegistry(const int32 NewSelectedRegistryTypeIndex) const
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return false;
	}

	const auto RowTypes = GetInventoryRowTypes();
	UScriptStruct* NewScriptStruct = RowTypes.IsValidIndex(NewSelectedRegistryTypeIndex) ? RowTypes[NewSelectedRegistryTypeIndex] : FAVVMDataTableRow::StaticStruct();

	const FText Title = NSLOCTEXT("AVVMEditorToolkit", "DataRegistry_ModalTitle", "Data Registry");
	const FText Msg = FText::FormatOrdered(NSLOCTEXT("AVVMEditorToolkit",
	                                                 "CreateNewDataRegistry",
	                                                 "Data Registry missing.\n\nPress Ok, and create a Data Registry. Make sure the RegistryType is:{0}, and ScriptStruct:{1}."),
	                                       FText::FromName(SelectedDataRegistryType.GetName()),
	                                       FText::FromString(GetNameSafe(NewScriptStruct)));

	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, Msg, Title);
	if (Response != EAppReturnType::Yes)
	{
		return false;
	}

	auto* Factory = NewObject<UAVVMDataRegistryFactory>();
	if (IsValid(Factory))
	{
		Factory->DataRegistryClass = UDataRegistry::StaticClass();
	}

	// @gdemers create Data Registry Asset.
	auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	auto* DataRegistry = AssetTools.Get().CreateAsset(FString::Printf(TEXT("DR_%s"), PROJECT_ASSET_PREFIX),
	                                                  SelectedAssetPath,
	                                                  UDataRegistry::StaticClass(),
	                                                  Factory);

	// @gdemers force open since we cannot default initialized it. the user should have read the modal, and know whats
	// expected from them at this point.
	AssetTools.Get().OpenEditorForAssets({DataRegistry});
	return IsValid(DataRegistry);
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
