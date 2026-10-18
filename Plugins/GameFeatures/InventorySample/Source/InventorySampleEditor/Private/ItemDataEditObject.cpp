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
#include "PropertyEditorModule.h"
#include "Components/VerticalBox.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Interfaces/IPluginManager.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"

TSharedPtr<SWidget> UItemDataEditObject::GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
                                                            const EAVVMDataEditorModes NewMode)
{
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
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		auto& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		FDetailsViewArgs DetailsViewArgs{};
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsViewArgs.bHideSelectionTip = true;

		BlueprintItemObjectPropertyView = EditModule.CreateDetailView(DetailsViewArgs);
		BlueprintItemObjectPropertyView->GetIsPropertyEditingEnabledDelegate().BindUObject(this, &UItemDataEditObject::OnEnable_BlueprintItemObjectDetailView);

		TableRowItemObjectPropertyView = EditModule.CreateDetailView(DetailsViewArgs);
		TableRowItemObjectPropertyView->GetIsPropertyEditingEnabledDelegate().BindUObject(this, &UItemDataEditObject::OnEnable_TableRowItemObjectDetailView);
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SAVVMEditorToolkitDataImporter)
				.Elements(GetItemTypes())
				.OnDataImporterSourceChanged_UObject(this, &UItemDataEditObject::OnItemTypeChanged)
				.DropDownHeaderText(NSLOCTEXT("AVVMEditorToolkit", "ItemTypeDropDownHeader", "Select an Item Type:"))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(8.0f)
				.Visibility_UObject(this, &UItemDataEditObject::OnVisibility_OnItemTypeChanged)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(12.f)
					.AutoHeight()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Left)
						.Font(FAppStyle::GetFontStyle("NormalFontBold"))
						.ToolTipText(NSLOCTEXT("AVVMEditorToolkit", "InputItemNameTooltip", "Note : Item Row Name are standardized under the hood."))
						.Text(NSLOCTEXT("AVVMEditorToolkit", "InputItemNameHeader", "Input a unique Item Row Name:"))
					]
					+ SVerticalBox::Slot()
					.Padding(12.f)
					.AutoHeight()
					[
						SNew(SEditableTextBox)
						.OnTextChanged_UObject(this, &UItemDataEditObject::OnTextChanged_ItemName)
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.Padding(20.0f, 1.0f, 20.0f, 1.0f)
				[
					BlueprintItemObjectPropertyView.ToSharedRef()
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.Padding(20.0f, 1.0f, 20.0f, 1.0f)
				[
					TableRowItemObjectPropertyView.ToSharedRef()
				]
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.ButtonColorAndOpacity(FLinearColor::Green)
				.IsEnabled_UObject(this, &UItemDataEditObject::OnEnable_GenerateButton)
				.OnClicked_UObject(this, &UItemDataEditObject::OnButtonClick_GenerateItem)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "ItemTypeDropDownHeader", "Generate"))
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

TArray<FName> UItemDataEditObject::GetItemTypes() const
{
	return {
			TEXT("Item"),
			TEXT("Attachment"),
			TEXT("Storage")
	};
}

EVisibility UItemDataEditObject::OnVisibility_OnItemTypeChanged() const
{
	return SelectedItemType.IsNone() ? EVisibility::Collapsed : EVisibility::Visible;
}

bool UItemDataEditObject::OnItemTypeChanged(const FName NewItemType,
                                            const int32 NewSelectedIndex)
{
	SelectedItemType = NewItemType;
	return (false == SelectedItemType.IsNone());
}

bool UItemDataEditObject::OnEnable_BlueprintItemObjectDetailView() const
{
	return false;
}

bool UItemDataEditObject::OnEnable_TableRowItemObjectDetailView() const
{
	return false;
}

bool UItemDataEditObject::OnEnable_GenerateButton() const
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

FText UItemDataEditObject::StandardizeItemObjectName() const
{
	FString PluginName{TEXT("Error")};
	
	TSharedPtr<IPlugin> OwningPlugin = IPluginManager::Get().GetModuleOwnerPlugin(FName(TEXT("InventorySampleEditor")));
	if (OwningPlugin.IsValid())
	{
		PluginName = OwningPlugin->GetName();
	}

	return FText::FromString(FString::Printf(TEXT("BP_%s_%s_ItemObject_%s_%s_C"), PROJECT_ASSET_PREFIX, *PluginName, *SelectedItemType.ToString(), *ItemRowName));
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

bool UItemDataEditObject::TryCreateNewItemObject()
{
	// if (!ensureAlwaysMsgf(SelectedDataTable.IsValid(),
	//                       TEXT("Data Table Output missing. Please select a target.")))
	// {
	// 	return false;
	// }
	//
	// const FString NewItemObjectName = MakeNewItemObjectName();
	// const bool bIsDuplicate = DoesItemObjectNameAlreadyExist(NewItemObjectName);
	// if (!ensureAlwaysMsgf((false == bIsDuplicate),
	//                       TEXT("Invalid ItemObject FName provided. Duplicated entry found.")))
	// {
	// 	return false;
	// }
	//
	// auto* NewItemDefinition = reinterpret_cast<FItemDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(SelectedDataTable.Get(), FName(NewItemObjectName)));
	// if (!ensureAlwaysMsgf(NewItemDefinition != nullptr,
	//                       TEXT("Failed to cast Row Type.")))
	// {
	// 	return false;
	// }
	//
	// bool bResult{false};
	// if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	// {
	// 	auto* Factory = NewObject<UItemDefinitionDataAssetFactory>();
	// 	if (IsValid(Factory))
	// 	{
	// 		Factory->DataAssetClass = UItemDefinitionDataAsset::StaticClass();
	// 	}
	//
	// 	auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	// 	const FString ModifiedAssetName = NewItemObjectName.Replace(TEXT("BP"), TEXT("DA"));
	//
	// 	bResult = ensureAlwaysMsgf(AssetTools.Get().CreateAssetWithDialogAsync(ModifiedAssetName,
	// 		                           AssetOutputPath,
	// 		                           UItemDefinitionDataAsset::StaticClass(),
	// 		                           Factory,
	// 		                           FAssetCreateComplete::CreateUObject(this, &UItemDataEditObject::OnDataAssetCreationCompleted, NewItemDefinition),
	// 		                           FAssetCreateCancelled::CreateUObject(this, &UItemDataEditObject::OnDataAssetCreationCancelled)),
	// 	                           TEXT("Failed to create new Data Asset."));
	// }

	return false;
}

void UItemDataEditObject::OnDataAssetCreationCompleted(UFactory* Factory,
                                                       UObject* NewObject,
                                                       FItemDefinitionDataTableRow* NewElement)
{
	if (!ensureAlwaysMsgf(NewElement != nullptr,
	                      TEXT("Invalid Row entry.")))
	{
		return;
	}

	NewElement->ItemDefinition = Cast<UItemDefinitionDataAsset>(NewObject);

	const int32 NewPhysicalGlobalId = GenerateNewPhysicalGlobalId();
	if (!ensureAlwaysMsgf((NewPhysicalGlobalId > 1) && (NewPhysicalGlobalId < INT32_MAX),
	                      TEXT("Failed to generate proper Physical Global Id.")))
	{
		return;
	}

	// TODO @gdemers run validation on new item object.
	// i.e do we have remaining space in the bit encoding for the item object type
	// does the name fits our requirements, etc...
}

void UItemDataEditObject::OnDataAssetCreationCancelled(UFactory* Factory)
{
}

int32 UItemDataEditObject::GenerateNewPhysicalGlobalId() const
{
	return INDEX_NONE;
}

TSharedPtr<SWidget> UItemGroupDataEditObject::GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
                                                                 const EAVVMDataEditorModes CurrentMode)
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

UItemDefinitionDataAssetFactory::UItemDefinitionDataAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UItemDefinitionDataAsset::StaticClass();
	SupportedWorkflows = static_cast<uint8>(EFactoryCreateWorkflow::Asynchronous);
}
