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
#include "AVVMDoesWidgetProvideModalMessaging.h"
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
#include "Misc/MessageDialog.h"
#include "SWidgets/AVVMEditorToolkitDataEditor.h"
#include "SWidgets/AVVMEditorToolkitDataTablePicker.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"
#include "SWidgets/AVVMEditorToolkitRequiredUserInput.h"
#include "SWidgets/AVVMEditorToolkitPluginHeader.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SScrollBox.h"
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
			SNew(SScrollBox)
			+ SScrollBox::Slot()
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
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::Step1", "Step 1: Enable or Disable automatic asset generation for each asset types."))
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
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::Step2", "Step 2: Select the Package Path where the automatically generated asset will be output."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SAVVMEditorToolkitOutputPathPicker)
					.OnPathSelected(this, &SInventoryToolkitWindow::OnOutputAssetPathSelected)
					.AssetPathHeaderText(NSLOCTEXT("AVVMEditorToolkit", "AssetPathHeader", "Double Click or Browse:"))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::Step3", "Step 3: Select to edit, or create a new DataRegistryType."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SAVVMEditorToolkitDataImporter)
					.Elements(GetInventoryDataRegistryTypes())
					.IsEnabled(this, &SInventoryToolkitWindow::IsEnabled_AssetPathSelectionChanged)
					.OnDataImporterSourceChanged(this, &SInventoryToolkitWindow::OnDataRegistrySelectionChanged)
					.DropDownHeaderText(NSLOCTEXT("AVVMEditorToolkit", "DataImporterHeader", "DataRegistryTypes:"))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::Step4", "Step 4: Select to edit, or create a new DataTable."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.FillHeight(1.f)
				[
					SAssignNew(DataTablePicker, SAVVMEditorToolkitDataTablePicker)
					.RegistryType(this, &SInventoryToolkitWindow::GetDataRegistryType)
					.IsEnabled(this, &SInventoryToolkitWindow::IsEnabled_DataTableAvailableChanged)
					.OnSelectionChanged(this, &SInventoryToolkitWindow::OnDataTableSelectionChanged)
					.PickerHeaderText(NSLOCTEXT("AVVMEditorToolkit", "PickerHeader", "DataTables:"))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SButton)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.ButtonColorAndOpacity(FLinearColor::Green)
					.IsEnabled(this, &SInventoryToolkitWindow::IsEnabled_DataRegistrySelectionChanged)
					.OnClicked(this, &SInventoryToolkitWindow::OnButtonClick_OpenWindow)
					.Text(this, &SInventoryToolkitWindow::OnTextChanged_CreateOrEdit)
				]
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
	if (!SelectedDataRegistry.IsValid() && (bCheckBoxStates.IsValidIndex(0/*UDataRegistry*/) && bCheckBoxStates[0]))
	{
		bResult = CreateDataRegistry();
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

void SInventoryToolkitWindow::OnOutputAssetPathSelected(const FString& NewPackagePath)
{
	SelectedPackagePath = NewPackagePath;
}

void SInventoryToolkitWindow::OnTextCommited_DataRegistryNameSet(const FText& NewAssetName,
                                                                 ETextCommit::Type CommitType)
{
	auto* Factory = NewObject<UAVVMDataRegistryFactory>();
	if (IsValid(Factory))
	{
		Factory->DataRegistryClass = UDataRegistry::StaticClass();

		FAVVMAssetFactoryContextArgs Params{};
		Params.AssetName = UAVVMEditorToolkitUtils::GetStandardizedName(SelectedPackagePath, TEXT("DR"), *NewAssetName.ToString());
		Params.PackagePath = SelectedPackagePath;
		Params.AssetFactory = Factory;
		Params.AssetClass = UDataRegistry::StaticClass();
		UAVVMEditorToolkitUtils::CreateOpenAsset<UDataRegistry>(Params);
	}
}

void SInventoryToolkitWindow::OnTextCommited_DataTaleNameSet(const FText& NewAssetName,
                                                             ETextCommit::Type CommitType)
{
	auto* Factory = NewObject<UDataTableFactory>();
	if (IsValid(Factory))
	{
		const auto RowTypes = GetInventoryRowTypes();
		Factory->Struct = RowTypes.IsValidIndex(SelectedRegistryTypeIndex) ? RowTypes[SelectedRegistryTypeIndex] : FAVVMDataTableRow::StaticStruct();

		FAVVMAssetFactoryContextArgs Params{};
		Params.AssetName = UAVVMEditorToolkitUtils::GetStandardizedName(SelectedPackagePath, TEXT("DT"), FString::Printf(TEXT("%s_%s"), *GetNameSafe(Factory->Struct), *NewAssetName.ToString()));
		Params.PackagePath = SelectedPackagePath;
		Params.AssetFactory = Factory;
		Params.AssetClass = UDataTable::StaticClass();
		SelectedDataTable = UAVVMEditorToolkitUtils::CreateOpenAsset<UDataTable>(Params);
	}

	if (ensureAlwaysMsgf(SelectedDataTable.IsValid(), TEXT("Failed to create new Data Table.")))
	{
		OpenWindow(SelectedDataTable, SelectedDataRegistryType, bShouldEditDataTable);
	}
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

bool SInventoryToolkitWindow::IsEnabled_AssetPathSelectionChanged() const
{
	return (false == SelectedPackagePath.IsEmpty());
}

bool SInventoryToolkitWindow::IsEnabled_DataTableAvailableChanged() const
{
	return (SelectedDataRegistryType.IsValid() && bHasDataTableAvailable);
}

bool SInventoryToolkitWindow::IsEnabled_DataRegistrySelectionChanged() const
{
	return SelectedDataRegistryType.IsValid();
}

TArray<FName> SInventoryToolkitWindow::GetAssetTypeToDefaultGenerate() const
{
	return {
			GetFNameSafe(UDataRegistry::StaticClass())
	};
}

TArray<FName> SInventoryToolkitWindow::GetInventoryDataRegistryTypes() const
{
	// @gdemers hard coded names for undefined DataRegistryTypes.
	return {
			TEXT("R_ItemGroup"),
			TEXT("R_Item"),
			TEXT("R_InventoryProviders"),
			TEXT("R_InventoryDependencyGraph")
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

FString SInventoryToolkitWindow::GetOutputPackagePath() const
{
	return SelectedPackagePath;
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

bool SInventoryToolkitWindow::CreateDataRegistry()
{
	const auto RowTypes = GetInventoryRowTypes();
	UScriptStruct* NewScriptStruct = RowTypes.IsValidIndex(SelectedRegistryTypeIndex) ? RowTypes[SelectedRegistryTypeIndex] : FAVVMDataTableRow::StaticStruct();

	FFormatNamedArguments Args{};
	Args.Add(TEXT("DataRegistryType"), FText::FromName(SelectedDataRegistryType.GetName()));
	Args.Add(TEXT("Values"), FText::FromString(GetNameSafe(NewScriptStruct)));
	const FText Msg = FText::Format(NSLOCTEXT("AVVMEditorToolkit", "CreateDataRegistry", "DataRegistry missing.\n\nPress Ok, and create a DataRegistry. Make sure the RegistryType is: {DataRegistryType},\nand ScriptStruct is: {Values}."), Args);

	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, Msg, NSLOCTEXT("AVVMEditorToolkit", "DataRegistry_ModalTitle", "DataRegistry"));
	if (Response != EAppReturnType::Yes)
	{
		return false;
	}

	FAVVMWindowContextArgs Params{};
	Params.Title = NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_DataRegistry", "DataRegistry");
	Params.Content = SAssignNew(PackagePathInput, SAVVMEditorToolkitRequiredUserInput)
		.bShouldCloseWindowOnCommit(true)
		.OnTextCommited(this, &SInventoryToolkitWindow::OnTextCommited_DataRegistryNameSet);
	
	Params.OnRequestDestroyWindowOverride = FRequestDestroyWindowOverride::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosedOverride);
	Params.OnWindowClosed = FOnWindowClosed::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosed);
	FloatingWindows.Add(UAVVMEditorToolkitUtils::OpenWindow(Params));
	return true;
}

bool SInventoryToolkitWindow::CreateDataTable()
{
	const auto RowTypes = GetInventoryRowTypes();
	UScriptStruct* NewScriptStruct = RowTypes.IsValidIndex(SelectedRegistryTypeIndex) ? RowTypes[SelectedRegistryTypeIndex] : FAVVMDataTableRow::StaticStruct();

	FFormatNamedArguments Args{};
	Args.Add(TEXT("Values"), FText::FromString(GetNameSafe(NewScriptStruct)));
	const FText Msg = FText::Format(NSLOCTEXT("AVVMEditorToolkit", "CreateDataTable", "Request new Data Table.\n\nPress Ok, and create a DataTable. Make sure the ScriptStruct is: {Values}."), Args);

	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, Msg, NSLOCTEXT("AVVMEditorToolkit", "DataTable_ModalTitle", "DataTable"));
	if (Response != EAppReturnType::Yes)
	{
		return false;
	}

	FAVVMWindowContextArgs Params{};
	Params.Title = NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_DataTable", "DataTable");
	Params.Content = SAssignNew(PackagePathInput, SAVVMEditorToolkitRequiredUserInput)
		.bShouldCloseWindowOnCommit(true)
		.OnTextCommited(this, &SInventoryToolkitWindow::OnTextCommited_DataTaleNameSet);

	Params.OnRequestDestroyWindowOverride = FRequestDestroyWindowOverride::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosedOverride);
	Params.OnWindowClosed = FOnWindowClosed::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosed);
	FloatingWindows.Add(UAVVMEditorToolkitUtils::OpenWindow(Params));
	return true;
}

void SInventoryToolkitWindow::EditDataTable()
{
	if (ensureAlwaysMsgf(SelectedDataTable.IsValid(), TEXT("Failed to create new Data Table.")))
	{
		OpenWindow(SelectedDataTable, SelectedDataRegistryType, bShouldEditDataTable);
	}
}

void SInventoryToolkitWindow::OpenWindow(const TWeakObjectPtr<UDataTable>& DataTable,
                                         const FName RegistryType,
                                         const bool bShouldOpenInEditMode)
{
	FAVVMWindowContextArgs Params{};
	Params.Title = NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_DataEditor", "DataEditor");
	Params.Content = SNew(SAVVMEditorToolkitDataEditor)
		.DataTablePackagePath(SelectedPackagePath)
		.DataEditorObject(GetDataEditObject(RegistryType))
		.OutputDataTable(DataTable)
		.Mode(bShouldOpenInEditMode ? EAVVMDataEditorModes::Edit : EAVVMDataEditorModes::Create);

	Params.OnRequestDestroyWindowOverride = FRequestDestroyWindowOverride::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosedOverride);
	Params.OnWindowClosed = FOnWindowClosed::CreateRaw(this, &SInventoryToolkitWindow::OnWindowClosed);
	FloatingWindows.Add(UAVVMEditorToolkitUtils::OpenWindow(Params));
}

void SInventoryToolkitWindow::OnWindowClosedOverride(const TSharedRef<SWindow>& PendingCloseWindow) const
{
	const auto* CtxWindow = dynamic_cast<IAVVMDoesWidgetProvideModalMessaging*>(&(PendingCloseWindow->GetContent().Get()));
	if (!ensureAlwaysMsgf(CtxWindow != nullptr, TEXT("Failed to cast to interface type.")))
	{
		return;
	}

	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, CtxWindow->GetModalMessage_OnClosure(), CtxWindow->GetModalTitle_OnClosure());
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
