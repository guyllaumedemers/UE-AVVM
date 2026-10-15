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
#include "PropertyEditorModule.h"
#include "Components/VerticalBox.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Factories/DataAssetFactory.h"
#include "Interfaces/IPluginManager.h"
#include "SWidgets/AVVMEditorToolkitDataTablePicker.h"
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"

TSharedPtr<SWidget> UItemDataEditObject::GetDataEditContent(const EAVVMDataEditorModes CurrentMode)
{
	TSharedPtr<SWidget> Out{nullptr};
	switch (CurrentMode)
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

	return SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SAVVMEditorToolkitDataImporter)
				.Elements(GetItemTypes())
				.OnDataImporterSourceChanged_UObject(this, &UItemDataEditObject::OnDataCategoryTypeChanged)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SAssignNew(OutputSelectorWidget, SAVVMEditorToolkitOutputPathPicker)
				.OnPathSelected_UObject(this, &UItemDataEditObject::OnAssetPathSelected)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SAVVMEditorToolkitDataTablePicker)
				.RegistryType(UInventorySettings::GetItemRegistryType())
				.OnSelectionChanged_UObject(this, &UItemDataEditObject::OnDataTableSelectionChanged)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SButton)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::Create", "Create ItemObject"))
				.TextStyle(FAppStyle::Get(), "DialogButtonText")
				.HAlign(HAlign_Center)
				.OnClicked_UObject(this, &UItemDataEditObject::OnButtonClick_GenerateElementId)
				.IsEnabled_UObject(this, &UItemDataEditObject::OnEnable_ButtonGenerateElementId)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
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
		];
}

TSharedPtr<SWidget> UItemDataEditObject::GetEditMode()
{
	return SNew(SImage)
		.ColorAndOpacity(FLinearColor::Green);
}

TArray<FName> UItemDataEditObject::GetItemTypes() const
{
	return {
			TEXT("Item"),
			TEXT("Attachment"),
			TEXT("Storage")
	};
}

FReply UItemDataEditObject::OnButtonClick_GenerateElementId()
{
	return TryCreateNewItemObject() ? FReply::Handled() : FReply::Unhandled();
}

bool UItemDataEditObject::OnDataCategoryTypeChanged(FName NewItemCategoryType)
{
	SelectedItemCategoryType = NewItemCategoryType;
	return (false == SelectedItemCategoryType.IsNone());
}

void UItemDataEditObject::OnTextFieldChanged(const FText& NewText)
{
	ItemRowName = NewText.ToString().TrimStartAndEnd();
}

void UItemDataEditObject::OnAssetPathSelected(const FString& NewPath)
{
	AssetOutputPath = NewPath;
}

bool UItemDataEditObject::OnEnable_ButtonGenerateElementId() const
{
	return (false == SelectedItemCategoryType.IsNone()) && (false == ItemRowName.IsEmpty()) && (false == AssetOutputPath.IsEmpty()) && SelectedDataTable.IsValid();
}

bool UItemDataEditObject::OnEnable_EditableText() const
{
	return (false == SelectedItemCategoryType.IsNone());
}

bool UItemDataEditObject::OnEnable_BlueprintItemObjectDetailView() const
{
	return OnEnable_ButtonGenerateElementId();
}

bool UItemDataEditObject::OnEnable_TableRowItemObjectDetailView() const
{
	return OnEnable_ButtonGenerateElementId();
}

void UItemDataEditObject::OnDataTableSelectionChanged(TWeakObjectPtr<UDataTable> NewSelection,
                                                      ESelectInfo::Type SelectInfo)
{
	if (ensureAlwaysMsgf(NewSelection.IsValid(),
	                     TEXT("Failed to retrieve reference to Data Table.")))
	{
		SelectedDataTable = NewSelection;
	}
}

FText UItemDataEditObject::GetItemObjectNameGenerated() const
{
	const FText OutputMsg = ItemRowName.IsEmpty() ? NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::GetItemObjectNameGenerated", "No Row Name provided.") : FText::FromString(MakeNewItemObjectName());
	return FText::Format(FTextFormat::FromString(TEXT("Output Row Name:{0}")), OutputMsg);
}

FText UItemDataEditObject::GetAssetOutputPath() const
{
	const FText OutputMsg = AssetOutputPath.IsEmpty() ? NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::GetAssetOutputPath", "No Asset path provided.") : FText::FromString(AssetOutputPath);
	return FText::Format(FTextFormat::FromString(TEXT("Asset Output Path:{0}")), OutputMsg);
}

FString UItemDataEditObject::MakeNewItemObjectName() const
{
	static const FString ProjPrefix{TEXT("AVVM")};

	FString PluginName{TEXT("Error")};

	TSharedPtr<IPlugin> OwningPlugin = IPluginManager::Get().GetModuleOwnerPlugin(FName(TEXT("InventorySampleEditor")));
	if (OwningPlugin.IsValid())
	{
		PluginName = OwningPlugin->GetName();
	}

	return FString::Printf(TEXT("BP_%s_%s_ItemObject_%s_%s_C"), *ProjPrefix, *PluginName, *SelectedItemCategoryType.ToString(), *ItemRowName);
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
	if (!ensureAlwaysMsgf(SelectedDataTable.IsValid(),
	                      TEXT("Data Table Output missing. Please select a target.")))
	{
		return false;
	}

	const FString NewItemObjectName = MakeNewItemObjectName();
	const bool bIsDuplicate = DoesItemObjectNameAlreadyExist(NewItemObjectName);
	if (!ensureAlwaysMsgf((false == bIsDuplicate),
	                      TEXT("Invalid ItemObject FName provided. Duplicated entry found.")))
	{
		return false;
	}

	auto* NewItemDefinition = reinterpret_cast<FItemDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(SelectedDataTable.Get(), FName(NewItemObjectName)));
	if (!ensureAlwaysMsgf(NewItemDefinition != nullptr,
	                      TEXT("Failed to cast Row Type.")))
	{
		return false;
	}

	bool bResult{false};
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		auto* Factory = NewObject<UItemDefinitionDataAssetFactory>();
		if (IsValid(Factory))
		{
			Factory->DataAssetClass = UItemDefinitionDataAsset::StaticClass();
		}

		auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		const FString ModifiedAssetName = NewItemObjectName.Replace(TEXT("BP"), TEXT("DA"));

		bResult = ensureAlwaysMsgf(AssetTools.Get().CreateAssetWithDialogAsync(ModifiedAssetName,
			                           AssetOutputPath,
			                           UItemDefinitionDataAsset::StaticClass(),
			                           Factory,
			                           FAssetCreateComplete::CreateUObject(this, &UItemDataEditObject::OnDataAssetCreationCompleted, NewItemDefinition),
			                           FAssetCreateCancelled::CreateUObject(this, &UItemDataEditObject::OnDataAssetCreationCancelled)),
		                           TEXT("Failed to create new Data Asset."));
	}

	return bResult;
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

TSharedPtr<SWidget> UItemGroupDataEditObject::GetDataEditContent(const EAVVMDataEditorModes CurrentMode)
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
