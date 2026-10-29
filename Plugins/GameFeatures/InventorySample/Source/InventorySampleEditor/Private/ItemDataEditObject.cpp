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
#include "AVVMEditorToolkitUtils.h"
#include "AVVMGameplaySettings.h"
#include "ContentBrowserModule.h"
#include "DataRegistrySubsystem.h"
#include "DataTableEditorUtils.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "SCheckBoxList.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Components/VerticalBox.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/AVVMActorUIDefinitionDataAsset.h"
#include "Data/AVVMDataRegistry.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Engine/DataTable.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/DataAssetFactory.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/MessageDialog.h"
#include "SWidgets/AVVMEditorToolkitDataTablePicker.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "SWidgets/AVVMEditorToolkitFloatingModal.h"
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"
#include "SWidgets/AVVMEditorToolkitSplitView.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScrollBox.h"

TSharedPtr<SWidget> UItemDataEditObject::GetDataEditContent(const FAVVMEditContextArgs& Params)
{
	SelectedDataTable_ItemDefinitions = Params.OutputDataTable;
	DataTablePackagePath = Params.PackagePath;

	TSharedPtr<SWidget> Out{Super::GetDataEditContent(Params)};
	switch (Params.CurrentMode)
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

FName UItemDataEditObject::GetRegistryType_ItemDefinitions() const
{
	return TEXT("R_Item");
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
					.OnTextChanged_UObject(this, &UItemDataEditObject::OnTextChanged_SetItemRowName)
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
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::Step4", "Step 4: Select to edit, or create the required DataTables."))
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
				.FillHeight(1.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SAssignNew(DataTablePicker_ActorUIDefinitions, SAVVMEditorToolkitDataTablePicker)
						.RegistryType(GetRegistryType_ActorUIDefinitions())
						.IsEnabled_UObject(this, &UItemDataEditObject::IsEnabled_OnItemTypeChanged)
						.OnSelectionChanged_UObject(this, &UItemDataEditObject::OnDataTableSelectionChanged_ActorUIDefinitions)
						.PickerHeaderText(NSLOCTEXT("AVVMEditorToolkit", "PickerHeader", "ActorUIDefinitions:"))
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
			SAssignNew(EditMode_DataVisualizer, SAVVMEditorToolkitDataVisualizer)
			.DataRegistryType(GetRegistryType_ItemDefinitions())
			.OnButtonClick_Create_UObject(this, &UItemDataEditObject::EditMode_OnButtonClick_Create)
			.OnButtonClick_Edit_UObject(this, &UItemDataEditObject::EditMode_OnButtonClick_Edit)
			.OnButtonClick_Delete_UObject(this, &UItemDataEditObject::EditMode_OnButtonClick_Delete)
		];
}

const TArray<FName>& UItemDataEditObject::GetAssetTypeToDefaultGenerate()
{
	static TArray<FName> Result = {
			GetFNameSafe(UDataRegistry::StaticClass()),
			GetFNameSafe(UDataTable::StaticClass())
	};

	return Result;
}

const TArray<FName>& UItemDataEditObject::GetItemTypes()
{
	static TArray<FName> Result = {
			TEXT("Item"),
			TEXT("Attachment"),
			TEXT("Storage")
	};

	return Result;
}

bool UItemDataEditObject::IsEnabled_AssetPathSelectionChanged() const
{
	return (false == SelectedPackagePath.IsEmpty());
}

bool UItemDataEditObject::IsEnabled_OnItemTypeChanged() const
{
	return (false == SelectedItemType.IsNone());
}

bool UItemDataEditObject::IsEnabled_GenerateButton() const
{
	return SelectedItemType.IsValid() && !ItemRowName.IsEmpty();
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

void UItemDataEditObject::OnDataTableSelectionChanged_ActorUIDefinitions(TWeakObjectPtr<UDataTable> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedDataTable_ActorUIDefinitions = NewSelection;
}

void UItemDataEditObject::OnTextChanged_SetItemRowName(const FText& NewItemName)
{
	ItemRowName = NewItemName.ToString();
}

FReply UItemDataEditObject::OnButtonClick_GenerateItem()
{
	if (!DataTablePicker_ActorIdentifiers.IsValid() ||
		!DataTablePicker_ActorDefinitions.IsValid() ||
		!DataTablePicker_ActorUIDefinitions.IsValid())
	{
		FFormatNamedArguments Args{};
		const FText Msg = FText::Format(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::Generate", "A data table selection is missing. Should we handle the creation here?"), Args);

		const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, Msg, NSLOCTEXT("AVVMEditorToolkit", "ItemEdit_ModalTitle", "ItemObject"));
		if (Response != EAppReturnType::Yes)
		{
			return FReply::Unhandled();
		}
	}

	// @gdemers user made the required selection, and is creating a new entry in each Data Table.
	const FReply Result = TryCreateNewItemObject() ? FReply::Handled() : FReply::Unhandled();
	OnRefreshAssetListRequested.ExecuteIfBound();

	struct FScoped
	{
		TSharedRef<SWidget> Widget{};

		~FScoped()
		{
			// @gdemers enforce closing window upon completing the action.
			TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(Widget);
			if (ensureAlwaysMsgf(ParentWindow.IsValid(), TEXT("Failed to find bound SWindow.")))
			{
				ParentWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride());
				ParentWindow->RequestDestroyWindow();
			}
		}
	};

	// @gdemers Unsafe, but required for order of execution.
	FScoped OnExit{OwningParent->AsShared()};
	return Result;
}

bool UItemDataEditObject::DoesItemObjectNameAlreadyExist(const FString& ItemObjectRowName) const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return true;
	}

	bool bResult{false};
	bResult |= (Subsystem->GetCachedItem<FItemDefinitionDataTableRow>(FDataRegistryId{GetRegistryType_ItemDefinitions(), FName(ItemObjectRowName)}) != nullptr);

	const FString ItemActorRowName = ItemObjectRowName.Replace(TEXT("ItemObject"), TEXT("ItemActor"));
	bResult |= (Subsystem->GetCachedItem<FAVVMActorIdentifierDataTableRow>(FDataRegistryId{GetRegistryType_ActorIdentifiers(), FName(ItemActorRowName)}) != nullptr);
	bResult |= (Subsystem->GetCachedItem<FAVVMActorDefinitionDataTableRow>(FDataRegistryId{GetRegistryType_ActorDefinitions(), FName(ItemActorRowName)}) != nullptr);

	const FString ItemActorUIRowName = ItemObjectRowName.Replace(TEXT("ItemObject"), TEXT("ItemActorUI"));
	bResult |= (Subsystem->GetCachedItem<FAVVMActorUIDefinitionDataTableRow>(FDataRegistryId{GetRegistryType_ActorUIDefinitions(), FName(ItemActorUIRowName)}) != nullptr);

	return bResult;
}

bool UItemDataEditObject::TryCreateNewItemObject()
{
	if (!ensureAlwaysMsgf(SelectedDataTable_ItemDefinitions.IsValid(),
	                      TEXT("Data Table Output missing. Please select a target.")))
	{
		return false;
	}

	const FString ItemObjectRowName = UAVVMEditorToolkitUtils::GetStandardizedName(SelectedPackagePath, TEXT("BP"), FString::Printf(TEXT("ItemObject_%s_%s_C"), *SelectedItemType.ToString(), *ItemRowName));
	const bool bIsDuplicate = DoesItemObjectNameAlreadyExist(ItemObjectRowName);
	if (!ensureAlwaysMsgf((false == bIsDuplicate), TEXT("Invalid ItemObject FName provided. Duplicated entry found.")))
	{
		FMessageDialog::Open(EAppMsgCategory::Error,
		                     EAppMsgType::Ok,
		                     NSLOCTEXT("AVVMEditorToolkit", "NotifyDataDuplicatedName", "Duplicated Name detected!\nChoose a different name."));
		return false;
	}

	auto* NewItemDefinition = reinterpret_cast<FItemDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(SelectedDataTable_ItemDefinitions.Get(), FName(ItemObjectRowName)));
	if (!ensureAlwaysMsgf(NewItemDefinition != nullptr, TEXT("Failed to cast Row Type.")))
	{
		return false;
	}

	auto* Factory = NewObject<UDataAssetFactory>();
	if (IsValid(Factory))
	{
		Factory->DataAssetClass = UItemDefinitionDataAsset::StaticClass();

		FAVVMEditorToolkitFactoryContextArgs Params{};
		Params.AssetName = ItemObjectRowName.Replace(TEXT("BP"), TEXT("DA"));
		Params.PackagePath = SelectedPackagePath;
		Params.AssetFactory = Factory;
		Params.AssetClass = UItemDefinitionDataAsset::StaticClass();
		auto* DataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UItemDefinitionDataAsset>(Params);
		if (ensureAlwaysMsgf(IsValid(DataAsset), TEXT("Failed to create New Data Asset.")))
		{
			NewItemDefinition->ItemDefinition = DataAsset;
			ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(SelectedDataTable_ItemDefinitions.Get()), TEXT("Failed to save Package."));
		}

		return TryCreateDependentAsset(DataAsset, SelectedItemType, ItemObjectRowName, SelectedPackagePath);
	}
	else
	{
		return false;
	}
}

bool UItemDataEditObject::TryCreateDependentAsset(UItemDefinitionDataAsset* ItemDefinitionDataAsset,
                                                  const FName NewItemType,
                                                  const FString& ItemObjectRowName,
                                                  const FString& NewPackagePath)
{
	if (!IsValid(ItemDefinitionDataAsset) || !ensureAlwaysMsgf((false == ItemObjectRowName.IsEmpty()), TEXT("Invalid Asset Name.")))
	{
		return false;
	}

	const FString ItemActorUIRowName = ItemObjectRowName.Replace(TEXT("ItemObject"), TEXT("ItemActorUI"));
	const FString ItemActorRowName = ItemObjectRowName.Replace(TEXT("ItemObject"), TEXT("ItemActor"));

	auto* BP_Factory = NewObject<UBlueprintFactory>();
	if (IsValid(BP_Factory))
	{
		BP_Factory->ParentClass = UItemObject::StaticClass();

		FAVVMEditorToolkitFactoryContextArgs BP_Params{};
		BP_Params.AssetName = ItemObjectRowName;
		BP_Params.PackagePath = NewPackagePath; // Blueprint Package path, and DA should be grouped together 
		BP_Params.AssetFactory = BP_Factory;
		BP_Params.AssetClass = UBlueprint::StaticClass();

		auto* BlueprintAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UBlueprint>(BP_Params);
		if (!ensureAlwaysMsgf(IsValid(BlueprintAsset), TEXT("Failed to create New Blueprint.")))
		{
			return false;
		}

		auto* ItemObject = Cast<UItemObject>(BlueprintAsset->GeneratedClass.GetDefaultObject());
		if (ensureAlwaysMsgf(IsValid(ItemObject), TEXT("Failed to create New Blueprint.")))
		{
			ItemObject->GetMutableItemSparseData()->ItemActorUIId = FDataRegistryId{GetRegistryType_ActorUIDefinitions(), FName(ItemActorUIRowName)};
			ItemObject->GetMutableItemSparseData()->ItemActorId = FDataRegistryId{GetRegistryType_ActorDefinitions(), FName(ItemActorRowName)};
			ItemDefinitionDataAsset->ItemObjectClass = ItemObject->GetClass();
		}
	}

	const bool bShouldCreateMissingDataRegistry = (bCheckBoxStates.IsValidIndex(0/*UDataRegistry*/) && bCheckBoxStates[0]);
	if (bShouldCreateMissingDataRegistry)
	{
		if (!UAVVMEditorToolkitUtils::DoesDataRegistryTypeExist(GetRegistryType_ActorIdentifiers()))
		{
			UAVVMEditorToolkitUtils::CreateMissingDataRegistry(FAVVMEditorToolkitScriptStructContextArgs{FAVVMActorIdentifierDataTableRow::StaticStruct(), GetRegistryType_ActorIdentifiers(), DataTablePackagePath});
		}

		if (!UAVVMEditorToolkitUtils::DoesDataRegistryTypeExist(GetRegistryType_ActorDefinitions()))
		{
			UAVVMEditorToolkitUtils::CreateMissingDataRegistry(FAVVMEditorToolkitScriptStructContextArgs{FAVVMActorDefinitionDataTableRow::StaticStruct(), GetRegistryType_ActorDefinitions(), DataTablePackagePath});
		}

		if (!UAVVMEditorToolkitUtils::DoesDataRegistryTypeExist(GetRegistryType_ActorUIDefinitions()))
		{
			UAVVMEditorToolkitUtils::CreateMissingDataRegistry(FAVVMEditorToolkitScriptStructContextArgs{FAVVMActorUIDefinitionDataTableRow::StaticStruct(), GetRegistryType_ActorUIDefinitions(), DataTablePackagePath});
		}
	}

	const bool bShouldCreateMissingDataTables = (bCheckBoxStates.IsValidIndex(1/*UDataTable*/) && bCheckBoxStates[1]);
	if (!SelectedDataTable_ActorIdentifiers.IsValid() && bShouldCreateMissingDataTables)
	{
		SelectedDataTable_ActorIdentifiers = UAVVMEditorToolkitUtils::CreateMissingDataTable(FAVVMEditorToolkitScriptStructContextArgs{FAVVMActorIdentifierDataTableRow::StaticStruct(), GetRegistryType_ActorIdentifiers(), DataTablePackagePath});
	}

	if (!SelectedDataTable_ActorDefinitions.IsValid() && bShouldCreateMissingDataTables)
	{
		SelectedDataTable_ActorDefinitions = UAVVMEditorToolkitUtils::CreateMissingDataTable(FAVVMEditorToolkitScriptStructContextArgs{FAVVMActorDefinitionDataTableRow::StaticStruct(), GetRegistryType_ActorDefinitions(), DataTablePackagePath});
	}

	if (!SelectedDataTable_ActorUIDefinitions.IsValid() && bShouldCreateMissingDataTables)
	{
		SelectedDataTable_ActorUIDefinitions = UAVVMEditorToolkitUtils::CreateMissingDataTable(FAVVMEditorToolkitScriptStructContextArgs{FAVVMActorUIDefinitionDataTableRow::StaticStruct(), GetRegistryType_ActorUIDefinitions(), DataTablePackagePath});
	}

	if (!ensureAlwaysMsgf(SelectedDataTable_ActorIdentifiers.IsValid() &&
	                      SelectedDataTable_ActorDefinitions.IsValid() &&
	                      SelectedDataTable_ActorUIDefinitions.IsValid(), TEXT("A Data Table is missing, and prevent us from proceeding.")))
	{
		FMessageDialog::Open(EAppMsgCategory::Error,
		                     EAppMsgType::Ok,
		                     NSLOCTEXT("AVVMEditorToolkit", "NotifyMissingDataTable", "DataTable missing!\nChoose or create the required DataTables."));

		return false;
	}

	bool bResult{true};
	bResult &= UAVVMEditorToolkitUtils::CreateActorIdentifierRow(FAVVMEditorDataTableActorIdentifierContextArgs{this, SelectedDataTable_ActorIdentifiers, FName(ItemActorRowName), NewItemType});
	bResult &= UAVVMEditorToolkitUtils::CreateActorDefinitionRow(FAVVMEditorDataTableOutputContextArgs{SelectedDataTable_ActorDefinitions, FName(ItemActorRowName), NewPackagePath});
	bResult &= UAVVMEditorToolkitUtils::CreateActorUIDefinitionRow(FAVVMEditorDataTableOutputContextArgs{SelectedDataTable_ActorUIDefinitions, FName(ItemActorUIRowName), NewPackagePath});
	return bResult;
}

int32 UItemDataEditObject::GenerateActorIdentifier(const FName ItemType) const
{
	const TArray<FName>& ItemTypes = GetItemTypes();
	if (!ItemTypes.Contains(ItemType))
	{
		return INDEX_NONE;
	}

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return INDEX_NONE;
	}

	const UDataRegistry* ActorIdentifierRegistry = Subsystem->GetRegistryForType(GetRegistryType_ActorIdentifiers());
	if (!IsValid(ActorIdentifierRegistry))
	{
		return INDEX_NONE;
	}

	TArray<const FAVVMActorIdentifierDataTableRow*> OutResults{};
	ActorIdentifierRegistry->GetAllItems(TEXT(""), OutResults);
	OutResults.Sort();

	int32 LowerBounds{INDEX_NONE};
	int32 UpperBounds{INDEX_NONE};

	if (ItemType.IsEqual(TEXT("Item")))
	{
		LowerBounds = GET_ITEM_PHYSICAL_ADDRESSING_OFFSET;
		UpperBounds = GET_ITEM_PHYSICAL_ADDRESSING_OFFSET + UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE);
	}
	else if (ItemType.IsEqual(TEXT("Attachment")))
	{
		LowerBounds = GET_ATTACHMENT_PHYSICAL_ADDRESSING_OFFSET;
		UpperBounds = GET_ITEM_PHYSICAL_ADDRESSING_OFFSET + UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE);
	}
	else
	{
		LowerBounds = GET_STORAGE_PHYSICAL_ADDRESSING_OFFSET;
		UpperBounds = GET_STORAGE_PHYSICAL_ADDRESSING_OFFSET + UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_STORAGE_POSITION_BIT_RANGE);
	}

	int32 NextValidId{INDEX_NONE};
	for (const FAVVMActorIdentifierDataTableRow* Row : OutResults)
	{
		if ((Row != nullptr) && ((Row->UniqueId >= LowerBounds) && (Row->UniqueId < UpperBounds)) && (Row->UniqueId > NextValidId))
		{
			NextValidId = Row->UniqueId;
		}
	}

	if (!ensureAlwaysMsgf(NextValidId != UpperBounds,
	                      TEXT("Failed to generate new unique Id. We have reach our limit. Reconfigure your bit encoding to int64, or remove deprecated entries.")))
	{
		return INDEX_NONE;
	}

	return (NextValidId == INDEX_NONE) ? (LowerBounds + 1) : (NextValidId + 1);
}

FReply UItemDataEditObject::EditMode_OnButtonClick_Create()
{
	FAVVMEditorToolkitWindowContextArgs Params{};
	Params.Title = NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_CreateNewItem", "Factory");
	Params.Content = SNew(SAVVMEditorToolkitFloatingModal)
		.Title(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::EditMode_CreateClicked_Title", "..."))
		.Message(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::EditMode_CreateClicked_Title", "..."))
		.Content(GetCreationMode());

	Params.OnRequestDestroyWindowOverride = FRequestDestroyWindowOverride::CreateUObject(this, &UItemDataEditObject::OnWindowClosedOverride);
	Params.OnWindowClosed = FOnWindowClosed::CreateUObject(this, &UItemDataEditObject::OnWindowClosed);
	FloatingWindows.Add(UAVVMEditorToolkitUtils::OpenWindow(Params));
	return FReply::Handled();
}

FReply UItemDataEditObject::EditMode_OnButtonClick_Edit()
{
	TArray<FName> OutRowNames{};
	if (!EditMode_DataVisualizer.IsValid() || !EditMode_DataVisualizer->GetSelectedItems(OutRowNames))
	{
		return FReply::Unhandled();
	}

	if (OutRowNames.IsEmpty())
	{
		return FReply::Unhandled();
	}
	
	if (OutRowNames.Num() > 1)
	{
		FFormatNamedArguments Args{};
		const FText Msg = FText::Format(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::EditMode_EditClicked", "Multiple Assets are selected for Edit. Do you want to continue?"), Args);

		const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo, Msg, NSLOCTEXT("AVVMEditorToolkit", "Edit_ModalTitle", "Edit"));
		if (Response != EAppReturnType::Yes)
		{
			return FReply::Unhandled();
		}
	}
	
	for (const FName RowName : OutRowNames)
	{
		EditDependentAsset(SelectedDataTable_ItemDefinitions.Get(), RowName);
	}

	return FReply::Handled();
}

FReply UItemDataEditObject::EditMode_OnButtonClick_Delete()
{
	TArray<FName> OutRowNames{};
	if (!EditMode_DataVisualizer.IsValid() || !EditMode_DataVisualizer->GetSelectedItems(OutRowNames))
	{
		return FReply::Unhandled();
	}

	if (OutRowNames.IsEmpty())
	{
		return FReply::Unhandled();
	}

	for (const FName RowName : OutRowNames)
	{
		DeleteDependentAsset(RowName);
	}

	return FReply::Handled();
}

void UItemDataEditObject::EditDependentAsset(const TWeakObjectPtr<UDataTable>& ItemDefinitionDataTable,
											 const FName RowName)
{
	const TArray<FDataRegistryId> DependentRegistryIds = GetDependentAssets(RowName);
	if (HasMissingDependencies(DependentRegistryIds) && ensureAlwaysMsgf(ItemDefinitionDataTable.IsValid(), TEXT("Invalid DataTable.")))
	{
		// @gdemers required to prevent FStringView from pointing to garbage memory.
		const FString PathName = ItemDefinitionDataTable->GetPathName();
		
		FStringView OutRoot{};
		FPackageName::SplitPackageName(PathName, &OutRoot, nullptr, nullptr, FPackageName::EPathFormatFlags::MountPointLeadingSlash);
		CreateMissingDependencies(RowName, FString{OutRoot});
	}

	FAVVMEditorToolkitWindowContextArgs Params{};
	Params.Title = NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_SplitView", "SplitView");
	Params.Content = SNew(SAVVMEditorToolkitSplitView)
		.RegistryType_ViewEditDenyList(TArray<FName>{GetRegistryType_ActorIdentifiers()})
		.bAreViewEditable(true)
		.DependentRegistryIds(DependentRegistryIds);

	Params.OnRequestDestroyWindowOverride = FRequestDestroyWindowOverride::CreateUObject(this, &UItemDataEditObject::OnWindowClosedOverride);
	Params.OnWindowClosed = FOnWindowClosed::CreateUObject(this, &UItemDataEditObject::OnWindowClosed);
	FloatingWindows.Add(UAVVMEditorToolkitUtils::OpenWindow(Params));
}

void UItemDataEditObject::CreateMissingDependencies(const FName RowName,
                                                    FString&& TopLevelPath)
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return;
	}

	const auto ItemActorRowName = FName(RowName.ToString().Replace(TEXT("ItemObject"), TEXT("ItemActor")));

	FTableRowBase OutRowBase_ActorIdentifier{};
	const bool bDoesExit_ActorIdentifier = Subsystem->GetCachedItemBP(FDataRegistryId{GetRegistryType_ActorIdentifiers(), ItemActorRowName}, OutRowBase_ActorIdentifier);
	if (!bDoesExit_ActorIdentifier)
	{
		SelectedDataTable_ActorIdentifiers = UAVVMEditorToolkitUtils::GetDependentDataTable(GetRegistryType_ActorIdentifiers(), TopLevelPath);
		UAVVMEditorToolkitUtils::CreateActorIdentifierRow({this, SelectedDataTable_ActorIdentifiers, ItemActorRowName, SelectedItemType});
	}

	FTableRowBase OutRowBase_ActorDefinition{};
	const bool bDoesExit_ActorDefinition = Subsystem->GetCachedItemBP(FDataRegistryId{GetRegistryType_ActorDefinitions(), ItemActorRowName}, OutRowBase_ActorDefinition);
	if (!bDoesExit_ActorDefinition || !UAVVMEditorToolkitUtils::DoesActorDefinitionRowHaveData(SelectedPackagePath, ItemActorRowName, OutRowBase_ActorDefinition))
	{
		SelectedDataTable_ActorDefinitions = UAVVMEditorToolkitUtils::GetDependentDataTable(GetRegistryType_ActorDefinitions(), TopLevelPath);
		UAVVMEditorToolkitUtils::CreateActorDefinitionRow({SelectedDataTable_ActorDefinitions, ItemActorRowName, SelectedPackagePath});
	}

	const auto ItemActorUIRowName = FName(RowName.ToString().Replace(TEXT("ItemObject"), TEXT("ItemActorUI")));

	FTableRowBase OutRowBase_ActorUIDefinition{};
	const bool bDoesExit_ActorUIDefinition = Subsystem->GetCachedItemBP(FDataRegistryId{GetRegistryType_ActorUIDefinitions(), ItemActorUIRowName}, OutRowBase_ActorUIDefinition);
	if (!bDoesExit_ActorUIDefinition || !UAVVMEditorToolkitUtils::DoesActorUIDefinitionRowHaveData(SelectedPackagePath, ItemActorUIRowName, OutRowBase_ActorUIDefinition))
	{
		SelectedDataTable_ActorUIDefinitions = UAVVMEditorToolkitUtils::GetDependentDataTable(GetRegistryType_ActorUIDefinitions(), TopLevelPath);
		UAVVMEditorToolkitUtils::CreateActorUIDefinitionRow({SelectedDataTable_ActorUIDefinitions, ItemActorUIRowName, SelectedPackagePath});
	}
}

void UItemDataEditObject::DeleteDependentAsset(const FName RowName)
{
}

TArray<FDataRegistryId> UItemDataEditObject::GetDependentAssets(const FName RowName) const
{
	const FName ItemActorRowName{RowName.ToString().Replace(TEXT("ItemObject"), TEXT("ItemActor"))};
	return {
			FDataRegistryId{GetRegistryType_ItemDefinitions(), RowName},
			FDataRegistryId{GetRegistryType_ActorIdentifiers(), ItemActorRowName},
			FDataRegistryId{GetRegistryType_ActorDefinitions(), ItemActorRowName},
			FDataRegistryId{GetRegistryType_ActorUIDefinitions(), FName(RowName.ToString().Replace(TEXT("ItemObject"), TEXT("ItemActorUI")))}
	};
}

bool UItemDataEditObject::HasMissingDependencies(const TArray<FDataRegistryId>& RegistryIds) const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return true;
	}

	bool bResult{true};
	for (const auto& RegistryId : RegistryIds)
	{
		bResult &= (Subsystem->GetCachedItem<FAVVMDataTableRow>(RegistryId) != nullptr);
	}

	return (false == bResult);
}

void UItemDataEditObject::OnPostWindowClosed() const
{
	if (EditMode_DataVisualizer.IsValid())
	{
		EditMode_DataVisualizer->UpdateDataVisualizer(GetRegistryType_ItemDefinitions());
	}

	if (DataTablePicker_ActorIdentifiers.IsValid())
	{
		DataTablePicker_ActorIdentifiers->UpdateDataTableCollection(GetRegistryType_ActorIdentifiers());
	}

	if (DataTablePicker_ActorDefinitions.IsValid())
	{
		DataTablePicker_ActorDefinitions->UpdateDataTableCollection(GetRegistryType_ActorDefinitions());
	}

	if (DataTablePicker_ActorUIDefinitions.IsValid())
	{
		DataTablePicker_ActorUIDefinitions->UpdateDataTableCollection(GetRegistryType_ActorUIDefinitions());
	}
}

TSharedPtr<SWidget> UItemGroupDataEditObject::GetDataEditContent(const FAVVMEditContextArgs& Params)
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
