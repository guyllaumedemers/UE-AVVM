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
#include "ItemDataEditObject.h"

#include "AssetToolsModule.h"
#include "AVVMEditorToolkitModule.h"
#include "AVVMEditorToolkitUtils.h"
#include "AVVMGameplaySettings.h"
#include "ContentBrowserModule.h"
#include "DataRegistrySubsystem.h"
#include "DataTableEditorUtils.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "SCheckBoxList.h"
#include "Components/VerticalBox.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/DataAssetFactory.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "SWidgets/AVVMEditorToolkitDataTablePicker.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScrollBox.h"

TSharedPtr<SWidget> UItemDataEditObject::GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
                                                            const EAVVMDataEditorModes NewMode)
{
	SelectedDataTable_ItemDefinitions = OutputDataTable;
	
	TSharedPtr<SWidget> Out{nullptr};
	switch (NewMode)
	{
		case EAVVMDataEditorModes::None:
			break;
		case EAVVMDataEditorModes::Create:
			Out = GetCreationMode();
			break;
		case EAVVMDataEditorModes::Edit:
			Out = GetEditMode();
			break;
	}

	return Out;
}

FText UItemDataEditObject::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UItemDataEditObject::DataEditor_ClosureMessage", "Do you want to close this Window. Any unsaved Items will be lost?");
}

FText UItemDataEditObject::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UItemDataEditObject::DataEditor_ClosureTitle", "UItemDataEditObject");
}

TSharedPtr<SWidget> UItemDataEditObject::GetCreationMode()
{
	SAssignNew(CheckBoxList, SCheckBoxList)
	.IncludeGlobalCheckBoxInHeaderRow(false)
	.OnItemCheckStateChanged_UObject(this, &UItemDataEditObject::OnCheckBoxStateChanged)
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

	return SNew(SOverlay)
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
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::Step1", "Step 1: Select the Package Path where the automatically generated asset will be output."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SAVVMEditorToolkitOutputPathPicker)
					.OnPathSelected_UObject(this, &UItemDataEditObject::OnOutputAssetPathSelected)
					.AssetPathHeaderText(NSLOCTEXT("AVVMEditorToolkit", "AssetPathHeader", "Double Click or Browse:"))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::ItemCreation::Step2", "Step 2: Select the Item Type to create."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SAVVMEditorToolkitDataImporter)
					.Elements(GetItemTypes())
					.IsEnabled_UObject(this, &UItemDataEditObject::IsEnabled_AssetPathSelectionChanged)
					.OnDataImporterSourceChanged_UObject(this, &UItemDataEditObject::OnItemTypeChanged)
					.DropDownHeaderText(NSLOCTEXT("AVVMEditorToolkit", "ItemTypeDropDownHeader", "Item Types:"))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::ItemCreation::Step3", "Step 3: Set a unique RowName for the new Item."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SEditableTextBox)
					.Justification(ETextJustify::Left)
					.Font(FAppStyle::GetFontStyle("NormalFontBold"))
					.OnTextChanged_UObject(this, &UItemDataEditObject::OnTextChanged_ItemName)
					.IsEnabled_UObject(this, &UItemDataEditObject::IsEnabled_OnItemTypeChanged)
					.HintText(NSLOCTEXT("AVVMEditorToolkit", "InputItemNameTooltip", "Note : Item Row Name are standardized under the hood."))
					.Text(NSLOCTEXT("AVVMEditorToolkit", "DefaultText", "..."))
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
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SAssignNew(DataTablePicker_ActorIdentifiers, SAVVMEditorToolkitDataTablePicker)
						.RegistryType(GetRegistryType_ActorIdentifiers())
						.IsEnabled_UObject(this, &UItemDataEditObject::IsEnabled_OnItemTypeChanged)
						.OnSelectionChanged_UObject(this, &UItemDataEditObject::OnDataTableSelectionChanged_ActorIdentifiers)
						.PickerHeaderText(NSLOCTEXT("AVVMEditorToolkit", "PickerHeader", "ActorIdentifiers:"))
					]
					+ SHorizontalBox::Slot()
					[
						SAssignNew(DataTablePicker_ActorDefinitions, SAVVMEditorToolkitDataTablePicker)
						.RegistryType(GetRegistryType_ActorDefinitions())
						.IsEnabled_UObject(this, &UItemDataEditObject::IsEnabled_OnItemTypeChanged)
						.OnSelectionChanged_UObject(this, &UItemDataEditObject::OnDataTableSelectionChanged_ActorDefinitions)
						.PickerHeaderText(NSLOCTEXT("AVVMEditorToolkit", "PickerHeader", "ActorDefinitions:"))
					]
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SButton)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.ButtonColorAndOpacity(FLinearColor::Green)
					.IsEnabled_UObject(this, &UItemDataEditObject::IsEnabled_GenerateButton)
					.OnClicked_UObject(this, &UItemDataEditObject::OnButtonClick_GenerateItem)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "ItemTypeDropDownHeader", "Generate"))
				]
			]
		];
}

TSharedPtr<SWidget> UItemDataEditObject::GetEditMode()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SAVVMEditorToolkitDataVisualizer)
		];
}

TArray<FName> UItemDataEditObject::GetAssetTypeToDefaultGenerate() const
{
	return {
			GetFNameSafe(UDataTable::StaticClass()),
			GetFNameSafe(UDataAsset::StaticClass()),
			GetFNameSafe(UBlueprint::StaticClass())
	};
}

TArray<FName> UItemDataEditObject::GetItemTypes() const
{
	return {
			TEXT("Item"),
			TEXT("Attachment"),
			TEXT("Storage")
	};
}

void UItemDataEditObject::OnCheckBoxStateChanged(int32 Index)
{
	if (bCheckBoxStates.IsValidIndex(Index))
	{
		bool& Value = bCheckBoxStates[Index];
		Value = !Value;
	}
}

void UItemDataEditObject::OnOutputAssetPathSelected(const FString& NewPackagePath)
{
	SelectedPackagePath = NewPackagePath;
}

bool UItemDataEditObject::IsEnabled_AssetPathSelectionChanged() const
{
	return (false == SelectedPackagePath.IsEmpty());
}

bool UItemDataEditObject::IsEnabled_OnItemTypeChanged() const
{
	return (false == SelectedItemType.IsNone());
}

bool UItemDataEditObject::OnItemTypeChanged(const FName NewItemType,
                                            const int32 NewSelectedIndex)
{
	SelectedItemType = NewItemType;
	return (false == SelectedItemType.IsNone());
}

void UItemDataEditObject::OnDataTableSelectionChanged_ActorIdentifiers(TWeakObjectPtr<UDataTable> NewSelection,
                                                                       ESelectInfo::Type SelectInfo)
{
	SelectedDataTable_ActorIdentifiers = NewSelection;
}

void UItemDataEditObject::OnDataTableSelectionChanged_ActorDefinitions(TWeakObjectPtr<UDataTable> NewSelection,
                                                                       ESelectInfo::Type SelectInfo)
{
	SelectedDataTable_ActorDefinitions = NewSelection;
}

bool UItemDataEditObject::IsEnabled_GenerateButton() const
{
	return SelectedItemType.IsValid() && !ItemRowName.IsEmpty();
}

void UItemDataEditObject::OnTextChanged_ItemName(const FText& NewItemName)
{
	ItemRowName = NewItemName.ToString();
}

FReply UItemDataEditObject::OnButtonClick_GenerateItem()
{
	return TryCreateNewItemObject() ? FReply::Handled() : FReply::Unhandled();
}

FString UItemDataEditObject::StandardizeItemObjectName() const
{
	FString PluginName{TEXT("Error")};

	TSharedPtr<IPlugin> OwningPlugin = IPluginManager::Get().GetModuleOwnerPlugin(FName(TEXT("InventorySampleEditor")));
	if (OwningPlugin.IsValid())
	{
		PluginName = OwningPlugin->GetName();
	}

	return FString::Printf(TEXT("BP_%s_%s_ItemObject_%s_%s_C"), PROJECT_ASSET_PREFIX, *PluginName, *SelectedItemType.ToString(), *ItemRowName);
}

bool UItemDataEditObject::DoesItemObjectNameAlreadyExist(const FString& NewValue) const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return true;
	}

	bool bResult{false};
	// @gdemers check if item object of name exist.
	bResult |= (Subsystem->GetCachedItem<FItemDefinitionDataTableRow>(FDataRegistryId{UInventorySettings::GetItemRegistryType(), FName(NewValue)}) != nullptr);

	// @gdemers check if actor of name exist.
	const FString ModifiedString = NewValue.Replace(TEXT("ItemObject"), TEXT("ItemActor"));
	bResult |= (Subsystem->GetCachedItem<FAVVMActorIdentifierDataTableRow>(FDataRegistryId{UAVVMGameplaySettings::GetActorIdentifierRegistryType(), FName(ModifiedString)}) != nullptr);
	bResult |= (Subsystem->GetCachedItem<FAVVMActorDefinitionDataTableRow>(FDataRegistryId{UAVVMGameplaySettings::GetActorDefinitionRegistryType(), FName(ModifiedString)}) != nullptr);

	return bResult;
}

bool UItemDataEditObject::TryCreateNewItemObject() const
{
	if (!ensureAlwaysMsgf(SelectedDataTable_ItemDefinitions.IsValid(),
	                      TEXT("Data Table Output missing. Please select a target.")))
	{
		return false;
	}

	const FString NewItemObjectName = StandardizeItemObjectName();
	const bool bIsDuplicate = DoesItemObjectNameAlreadyExist(NewItemObjectName);
	if (!ensureAlwaysMsgf((false == bIsDuplicate), TEXT("Invalid ItemObject FName provided. Duplicated entry found.")))
	{
		FMessageDialog::Open(EAppMsgCategory::Error,
		                     EAppMsgType::Ok,
		                     NSLOCTEXT("AVVMEditorToolkit", "NotifyDataDuplicatedName", "Duplicated Name detected!\nChoose a different name."));
		return false;
	}

	auto* NewItemDefinition = reinterpret_cast<FItemDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(SelectedDataTable_ItemDefinitions.Get(), FName(NewItemObjectName)));
	if (!ensureAlwaysMsgf(NewItemDefinition != nullptr, TEXT("Failed to cast Row Type.")))
	{
		return false;
	}

	auto* Factory = NewObject<UDataAssetFactory>();
	if (IsValid(Factory))
	{
		Factory->DataAssetClass = UItemDefinitionDataAsset::StaticClass();

		FAVVMAssetFactoryContextArgs Params{};
		Params.AssetName = NewItemObjectName.Replace(TEXT("BP"), TEXT("DA"));
		Params.PackagePath = SelectedPackagePath;
		Params.AssetFactory = Factory;
		Params.AssetClass = UItemDefinitionDataAsset::StaticClass();
		auto* DataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UItemDefinitionDataAsset>(Params);
		if (ensureAlwaysMsgf(IsValid(DataAsset), TEXT("Failed to create New Data Asset.")))
		{
			NewItemDefinition->ItemDefinition = DataAsset;
		}

		return TryCreateDependentAsset(DataAsset, NewItemObjectName);
	}
	else
	{
		return false;
	}
}

bool UItemDataEditObject::TryCreateDependentAsset(UItemDefinitionDataAsset* ItemDefinitionDataAsset,
                                                  const FString& AssetName) const
{
	if (!IsValid(ItemDefinitionDataAsset) || !ensureAlwaysMsgf((false == AssetName.IsEmpty()),
	                                                           TEXT("Invalid Asset Name.")))
	{
		return false;
	}
	
	auto* BP_Factory = NewObject<UBlueprintFactory>();
	if (IsValid(BP_Factory))
	{
		BP_Factory->ParentClass = UItemObject::StaticClass();

		FAVVMAssetFactoryContextArgs Params{};
		Params.AssetName = AssetName;
		Params.PackagePath = SelectedPackagePath;
		Params.AssetFactory = BP_Factory;
		Params.AssetClass = UItemDefinitionDataAsset::StaticClass();
		auto* ItemObject = UAVVMEditorToolkitUtils::CreateOpenAsset<UItemObject>(Params);
		if (ensureAlwaysMsgf(IsValid(ItemObject), TEXT("Failed to create New Blueprint.")))
		{
			ItemDefinitionDataAsset->ItemObjectClass = ItemObject->GetClass();
		}
	}

	return true;
}

int32 UItemDataEditObject::GenerateNewPhysicalGlobalId() const
{
	return INDEX_NONE;
}

TSharedPtr<SWidget> UItemGroupDataEditObject::GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
                                                                 const EAVVMDataEditorModes NewMode)
{
	return SNew(SImage)
		.ColorAndOpacity(FLinearColor::Blue);
}

FText UItemGroupDataEditObject::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UItemGroupDataEditObject::DataEditor_ClosureMessage", "Do you want to close this Window. Any unsaved ItemGroups will be lost?");
}

FText UItemGroupDataEditObject::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UItemGroupDataEditObject::DataEditor_ClosureTitle", "UItemGroupDataEditObject");
}
