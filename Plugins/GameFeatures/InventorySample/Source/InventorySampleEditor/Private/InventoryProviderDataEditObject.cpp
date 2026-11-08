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

#include "DataRegistry.h"
#include "PropertyCustomizationHelpers.h"
#include "SCheckBoxList.h"
#include "SGameplayTagContainerCombo.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedPtr<SWidget> UFtueInventoryProviderDataEditObject::GetDataEditContent(const FAVVMEditContextArgs& Params)
{
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

		DefaultInventory = TStrongObjectPtr(NewObject<UItemArrayBuilder>());
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
					SNew(SClassPropertyEntryBox)
					.MetaClass(AActor::StaticClass())
					.AllowNone(true)
					.AllowAbstract(false)
					.SelectedClass_UObject(this, &UFtueInventoryProviderDataEditObject::OnGetClass)
					.OnSetClass_UObject(this, &UFtueInventoryProviderDataEditObject::OnSetClass)
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
						.Text(NSLOCTEXT("AVVMEditorToolkit", "LoadoutSlotTags", "Can Inventory Provider equip Items in loadout?"))
					]
				]
				+ SVerticalBox::Slot()
				.Padding(12.f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FAppStyle::Get(), "LargeText")
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::InventoryProvider::Step4", "Step 4: Define the Loadout Slots of your Provider inventory."))
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
					.Text(NSLOCTEXT("AVVMEditorToolkit", "Guide::InventoryProvider::Step4", "Step 4: Define the Loadout Slots of your Provider inventory."))
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
		return !LoadoutSlots.IsEmpty() && DefaultInventory.IsValid() && !DefaultInventory->DefaultItems.IsEmpty();
	}
	else
	{
		return DefaultInventory.IsValid() && !DefaultInventory->DefaultItems.IsEmpty();
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

FReply UFtueInventoryProviderDataEditObject::OnButtonClick_GenerateItem()
{
	return FReply::Unhandled();
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
