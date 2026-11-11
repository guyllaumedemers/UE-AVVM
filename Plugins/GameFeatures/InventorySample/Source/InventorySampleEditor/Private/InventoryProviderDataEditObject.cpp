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
#include "InventoryProviderDataEditObject.h"

#include "AVVMEditorToolkitUtils.h"
#include "ContentBrowserModule.h"
#include "DataRegistrySubsystem.h"
#include "DataTableEditorUtils.h"
#include "IContentBrowserSingleton.h"
#include "InventoryProvider.h"
#include "PropertyCustomizationHelpers.h"
#include "SCheckBoxList.h"
#include "SGameplayTagContainerCombo.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/InventoryProviderTableRow.h"
#include "Factories/BlueprintFactory.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedPtr<SWidget> UFtueInventoryProviderDataEditObject::GetDataEditContent(const FAVVMEditContextArgs& Params)
{
	SelectedDataTable_InventoryProviderDefinitions = Params.OutputDataTable;
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

int32 UFtueInventoryProviderDataEditObject::GenerateActorIdentifier(const FName ActorType) const
{
	const TArray<FName>& ActorTypes = GetActorTypes();
	if (!ActorTypes.Contains(ActorType))
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

	if (ActorType.IsEqual(TEXT("Playable")))
	{
		LowerBounds = GET_PLAYABLE_CHARACTER_PHYSICAL_ADDRESSING_OFFSET;
		UpperBounds = GET_PLAYABLE_CHARACTER_PHYSICAL_ADDRESSING_OFFSET + UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE);
	}
	else if (ActorType.IsEqual(TEXT("NPC")))
	{
		LowerBounds = GET_NPC_PHYSICAL_ADDRESSING_OFFSET;
		UpperBounds = GET_NPC_PHYSICAL_ADDRESSING_OFFSET + UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE);
	}
	else
	{
		LowerBounds = GET_WORLD_ACTOR_PHYSICAL_ADDRESSING_OFFSET;
		UpperBounds = GET_WORLD_ACTOR_PHYSICAL_ADDRESSING_OFFSET + UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE);
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

FText UFtueInventoryProviderDataEditObject::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UFtueInventoryProviderDataEditObject::DataEditor_ClosureMessage", "Do you want to close this Window. Any unsaved InventoryProviders will be lost?");
}

FText UFtueInventoryProviderDataEditObject::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UFtueInventoryProviderDataEditObject::DataEditor_ClosureTitle", "UFtueInventoryProviderDataEditObject");
}

TSharedPtr<SWidget> UFtueInventoryProviderDataEditObject::GetCreationMode()
{
	TSharedPtr<IDetailsView> ItemArrayDetailView{nullptr};
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor") && !DefaultInventory.IsValid())
	{
		FDetailsViewArgs DetailsViewArgs{};
		DetailsViewArgs.bAllowSearch = true;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsViewArgs.bAllowFavoriteSystem = false;

		DefaultInventory = TStrongObjectPtr(NewObject<UInventoryProviderArrayBuilder>());
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		ItemArrayDetailView = PropertyModule.CreateDetailView(DetailsViewArgs);
		ItemArrayDetailView->SetObject(DefaultInventory.Get());
		ItemArrayDetailView->GetIsPropertyEditingEnabledDelegate().BindUObject(this, &UFtueInventoryProviderDataEditObject::IsEnabled_OnActorClassChanged);
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
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::InventoryProvider::Step2", "Step 2: Select the Actor Type to create a Provider for."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(SClassPropertyEntryBox)
						.MetaClass(AActor::StaticClass())
						.RequiredInterface(UInventoryProvider::StaticClass())
						.IsBlueprintBaseOnly(true)
						.AllowAbstract(false)
						.AllowNone(true)
						.SelectedClass_UObject(this, &UFtueInventoryProviderDataEditObject::OnGetClass)
						.OnSetClass_UObject(this, &UFtueInventoryProviderDataEditObject::OnSetClass)
					]
					+ SHorizontalBox::Slot()
					.Padding(FMargin{12.f, 0.f})
					.AutoWidth()
					[
						SNew(SButton)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.IsEnabled_UObject(this, &UFtueInventoryProviderDataEditObject::IsEnabled_OnActorClassMissing)
						.OnClicked_UObject(this, &UFtueInventoryProviderDataEditObject::OnButtonClick_GenerateActorClass)
						.Text(NSLOCTEXT("AVVMEditorToolkit", "GenerateProviderClass", "Create Inventory Provider Class"))
					]
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::InventoryProvider::Step3", "Step 3: Define if your Provider should default equip items."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(EHorizontalAlignment::HAlign_Left)
					.AutoWidth()
					[
						SNew(SCheckBox)
						.IsEnabled_UObject(this, &UFtueInventoryProviderDataEditObject::IsEnabled_OnActorClassChanged)
						.IsChecked_UObject(this, &UFtueInventoryProviderDataEditObject::GetOnCheckboxState)
						.OnCheckStateChanged_UObject(this, &UFtueInventoryProviderDataEditObject::OnCheckBoxStateChanged)
					]
					+ SHorizontalBox::Slot()
					.HAlign(EHorizontalAlignment::HAlign_Left)
					.AutoWidth()
					[
						SNew(STextBlock)
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						.Text(NSLOCTEXT("AVVMEditorToolkit", "LoadoutSlotTags", "Does Provider equip items?"))
					]
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::InventoryProvider::Step4", "Step 4 (Optional): Define the Loadout Slots of your Provider inventory."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SGameplayTagContainerCombo)
					.Filter(TEXT("InventorySample.Item.Slot"))
					.IsEnabled_UObject(this, &UFtueInventoryProviderDataEditObject::IsEnabled_OnSlotTagRequirementsChanged)
					.TagContainer_UObject(this, &UFtueInventoryProviderDataEditObject::GetTagContainer)
					.OnTagCleared_UObject(this, &UFtueInventoryProviderDataEditObject::OnTagCleared)
					.OnTagContainerChanged_UObject(this, &UFtueInventoryProviderDataEditObject::OnTagComboChanged)
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::InventoryProvider::Step5", "Step 5: Define the default inventory content your Provider holds on FTUE."))
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					ItemArrayDetailView.ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(SButton)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.ButtonColorAndOpacity(FLinearColor::Green)
					.IsEnabled_UObject(this, &UFtueInventoryProviderDataEditObject::IsEnabled_GenerateButton)
					.OnClicked_UObject(this, &UFtueInventoryProviderDataEditObject::OnButtonClick_GenerateItem)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "ItemTypeDropDownHeader", "Generate"))
				]
			]
		];
}

TSharedPtr<SWidget> UFtueInventoryProviderDataEditObject::GetEditMode()
{
	return SNew(SOverlay);
}

const TArray<FName>& UFtueInventoryProviderDataEditObject::GetActorTypes()
{
	static TArray<FName> Result = {
			TEXT("Playable"),
			TEXT("NPC"),
			TEXT("World")
	};

	return Result;
}

FName UFtueInventoryProviderDataEditObject::GetActorType(const UClass* ActorClass)
{
	if (!IsValid(ActorClass))
	{
		return NAME_None;
	}

	const TArray<FName>& ActorTypes = GetActorTypes();
	// @gdemers GetDefaultObject<APawn> will assert on failure to cast to non-Pawn actors.
	const auto* CDO = ActorClass->GetDefaultObject<UObject>();
	const auto* PawnCDO = IsValid(CDO) && CDO->IsA<APawn>() ? Cast<APawn>(CDO) : nullptr;

	if (!IsValid(PawnCDO))
	{
		return ActorTypes[2/*World*/];
	}
	else if (IsValid(PawnCDO->AIControllerClass))
	{
		return ActorTypes[1/*NPC*/];
	}
	else
	{
		return ActorTypes[0/*Playable*/];
	}
}

bool UFtueInventoryProviderDataEditObject::IsEnabled_OnActorClassMissing() const
{
	return (false == ActorClass.IsValid());
}

bool UFtueInventoryProviderDataEditObject::IsEnabled_OnActorClassChanged() const
{
	return ActorClass.IsValid();
}

bool UFtueInventoryProviderDataEditObject::IsEnabled_OnSlotTagRequirementsChanged() const
{
	return (CheckboxState == ECheckBoxState::Checked);
}

bool UFtueInventoryProviderDataEditObject::IsEnabled_GenerateButton() const
{
	if ((CheckboxState == ECheckBoxState::Checked))
	{
		return !LoadoutSlots.IsEmpty() && DefaultInventory.IsValid() && !DefaultInventory->FtueInventory.IsEmpty();
	}
	else
	{
		return DefaultInventory.IsValid() && !DefaultInventory->FtueInventory.IsEmpty();
	}
}

const UClass* UFtueInventoryProviderDataEditObject::OnGetClass() const
{
	return ActorClass.Get();
}

void UFtueInventoryProviderDataEditObject::OnSetClass(const UClass* NewSelectedClass)
{
	ActorClass = NewSelectedClass;
}

ECheckBoxState UFtueInventoryProviderDataEditObject::GetOnCheckboxState() const
{
	return CheckboxState;
}

void UFtueInventoryProviderDataEditObject::OnCheckBoxStateChanged(ECheckBoxState NewState)
{
	CheckboxState = NewState;
}

void UFtueInventoryProviderDataEditObject::OnTagComboChanged(const FGameplayTagContainer& TagContainer)
{
	LoadoutSlots = TagContainer;
}

FGameplayTagContainer UFtueInventoryProviderDataEditObject::GetTagContainer() const
{
	return LoadoutSlots;
}

void UFtueInventoryProviderDataEditObject::OnTagCleared()
{
	LoadoutSlots.Reset();
}

FReply UFtueInventoryProviderDataEditObject::OnButtonClick_GenerateActorClass()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FReply::Unhandled();
	}

	auto* BlueprintFactory = NewObject<UBlueprintFactory>();
	if (IsValid(BlueprintFactory))
	{
		BlueprintFactory->ParentClass = AActor::StaticClass();
	}

	// @gdemers we don't define the asset name since we are opening a context window.
	const FString ProviderRowName = UAVVMEditorToolkitUtils::GetStandardizedName(DataTablePackagePath, TEXT("BP"), {});
	const FString PackagePath = UAVVMEditorToolkitUtils::CreateTopLevelDirectory(DataTablePackagePath, DATA_ASSET_DIRECTORY); // Blueprint Package path, and DA should be grouped together 

	auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	auto* NewBlueprint = Cast<UBlueprint>(AssetTools.Get().CreateAssetWithDialog(ProviderRowName, PackagePath, UBlueprint::StaticClass(), BlueprintFactory, FName("Create Inventory Provider Actor Blueprint")));
	if (IsValid(NewBlueprint))
	{
		FBPInterfaceDescription ProviderInterface{};
		ProviderInterface.Interface = UInventoryProvider::StaticClass();
		NewBlueprint->ImplementedInterfaces.Add({MoveTemp(ProviderInterface)});
		ensureAlwaysMsgf(UEditorAssetLibrary::SaveLoadedAsset(NewBlueprint), TEXT("Failed to save Package."));

		AssetTools.Get().OpenEditorForAssets({NewBlueprint});
		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

FReply UFtueInventoryProviderDataEditObject::OnButtonClick_GenerateItem()
{
	return TryCreateNewInventoryProvider() ? FReply::Handled() : FReply::Unhandled();
}

bool UFtueInventoryProviderDataEditObject::DoesInventoryProviderAlreadyExist(const FName InventoryProviderRowName) const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return true;
	}

	const bool bResult = (Subsystem->GetCachedItem<FAVVMActorIdentifierDataTableRow>(FDataRegistryId{GetRegistryType_ActorIdentifiers(), InventoryProviderRowName}) != nullptr);
	return bResult;
}

bool UFtueInventoryProviderDataEditObject::TryCreateNewInventoryProvider()
{
	if (!ActorClass.IsValid() || !ensureAlwaysMsgf(SelectedDataTable_InventoryProviderDefinitions.IsValid(),
	                                               TEXT("Data Table Output missing. Please select a target.")))
	{
		return false;
	}

	const FName RowName{ActorClass->GetFName()};

	const bool bIsDuplicate = DoesInventoryProviderAlreadyExist(RowName);
	if (!ensureAlwaysMsgf((false == bIsDuplicate), TEXT("Invalid Actor Class selected. Duplicated InventoryProvider entry found.")))
	{
		FMessageDialog::Open(EAppMsgCategory::Error,
		                     EAppMsgType::Ok,
		                     NSLOCTEXT("AVVMEditorToolkit", "NotifyProviderDuplicatedName", "Duplicated entry detected!\nChoose a different Actor Class."));
		return false;
	}

	const bool bResult = UAVVMEditorToolkitUtils::CreateActorIdentifierRow(FAVVMEditorDataTableActorIdentifierContextArgs{this, SelectedDataTable_InventoryProviderDefinitions, RowName, GetActorType(ActorClass.Get())});
	if (!ensureAlwaysMsgf(bResult, TEXT("Failed to create new InventoryProvider UniqueId entry.")))
	{
		return false;
	}

	auto* NewInventoryProvider = reinterpret_cast<FFtue_InventoryProviderTableRow*>(FDataTableEditorUtils::AddRow(SelectedDataTable_InventoryProviderDefinitions.Get(), RowName));
	if (!ensureAlwaysMsgf(NewInventoryProvider != nullptr, TEXT("Failed to cast Row Type.")))
	{
		return false;
	}

	const bool bRequireSlotDefinition = (GetOnCheckboxState() == ECheckBoxState::Checked);
	NewInventoryProvider->ParentActorIdentifierId = {GetRegistryType_ActorIdentifiers(), RowName};
	NewInventoryProvider->bCanInventoryProviderEquipItems = bRequireSlotDefinition;
	// TODO @gdemers Rework so defining those properties is easier for user, and we take them by the hand as they add new entries.
	NewInventoryProvider->ParentFtueInventory = DefaultInventory.IsValid() ? DefaultInventory->FtueInventory : TMap<TSoftClassPtr<UItemObject>, FFtue_ItemObjectProperties>{};
	if (bRequireSlotDefinition)
	{
		NewInventoryProvider->DefaultSlotTags = LoadoutSlots.GetGameplayTagArray();
	}

	ensureAlwaysMsgf(UEditorAssetLibrary::SaveLoadedAsset(SelectedDataTable_InventoryProviderDefinitions.Get()), TEXT("Failed to save Package."));
	return true;
}

TSharedPtr<SWidget> UStubDataInventoryDependencyGraphEditObject::GetDataEditContent(const FAVVMEditContextArgs& Params)
{
	return SNew(SImage)
		.ColorAndOpacity(FLinearColor::Red);
}

FText UStubDataInventoryDependencyGraphEditObject::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UStubDataInventoryDependencyGraphEditObject::DataEditor_ClosureMessage", "Do you want to close this Window. Any unsaved Dependency Graph Entries will be lost?");
}

FText UStubDataInventoryDependencyGraphEditObject::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UStubDataInventoryDependencyGraphEditObject::DataEditor_ClosureTitle", "UStubDataInventoryDependencyGraphEditObject");
}
