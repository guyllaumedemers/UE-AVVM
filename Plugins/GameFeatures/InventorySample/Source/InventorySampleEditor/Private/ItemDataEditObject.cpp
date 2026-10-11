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

#include "PropertyEditorModule.h"
#include "Components/VerticalBox.h"
#include "Modules/ModuleManager.h"
#include "SWidgets/AVVMEditorToolkitDataImporter.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

TSharedPtr<SWidget> UItemDataEditObject::GetDataEditContent(const EAVVMDataEditorModes CurrentMode)
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

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
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
				.Text(NSLOCTEXT("AVVMEditorToolkit", "UItemGroupDataEditObject::Hint", "Select the ItemObject category type to create!"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SAVVMEditorToolkitDataImporter)
				.Elements(GetItemTypes())
				.OnDataImporterSourceChanged_UObject(this, &UItemDataEditObject::OnDataCategoryTypeChanged)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SEditableTextBox)
				.Justification(ETextJustify::Center)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::SetItemName", "Input new ItemObject FName"))
				.HintText(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::SetItemName_Hint", "ItemObject FName must follow a strict naming convention. A new entry with _ItemObject will be created under the hood for your new Object."))
				.OnTextChanged_UObject(this, &UItemDataEditObject::OnTextFieldChanged)
				.IsEnabled_UObject(this, &UItemDataEditObject::OnEnable_EditableText)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SButton)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "UItemDataEditObject::GenerateElementId", "Generate ItemObject unique Id"))
				.TextStyle(FAppStyle::Get(), "DialogButtonText")
				.HAlign(HAlign_Center)
				.OnClicked_UObject(this, &UItemDataEditObject::OnButtonClick_GenerateElementId)
				.IsEnabled_UObject(this, &UItemDataEditObject::OnEnable_ButtonGenerateElementId)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.Text(GetItemObjectNameGenerated())
				.Visibility_UObject(this, &UItemDataEditObject::OnItemObjectNameGenerated)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.Padding(10.f)
				[
					BlueprintItemObjectPropertyView.ToSharedRef()
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					TableRowItemObjectPropertyView.ToSharedRef()
				]
			]
		];
}

FText UItemDataEditObject::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UItemDataEditObject::DataEditor_ClosureMessage", "Do you want to close this Window. Any unsaved Items will be lost?");
}

FText UItemDataEditObject::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "UItemDataEditObject::DataEditor_ClosureTitle", "UItemDataEditObject");
}

TArray<FName> UItemDataEditObject::GetItemTypes() const
{
	return {
			TEXT("Item"),
			TEXT("Attachment"),
			TEXT("Storage")
	};
}

FReply UItemDataEditObject::OnButtonClick_GenerateElementId() const
{
	return TryCreateNewItemObject() ? FReply::Handled() : FReply::Unhandled();
}

bool UItemDataEditObject::OnDataCategoryTypeChanged(FName NewItemCategoryType)
{
	SelectedItemCategoryType = NewItemCategoryType;
	return true;
}

void UItemDataEditObject::OnTextFieldChanged(const FText& NewText)
{
	ItemRowName = NewText.ToString().TrimStartAndEnd();
}

bool UItemDataEditObject::OnEnable_ButtonGenerateElementId() const
{
	return (false == SelectedItemCategoryType.IsNone()) && (false == ItemRowName.IsEmpty());
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

EVisibility UItemDataEditObject::OnItemObjectNameGenerated() const
{
	// TODO @gdemers further define visibility requirements
	return ItemRowName.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

FText UItemDataEditObject::GetItemObjectNameGenerated() const
{
	// TODO @gdemers make the system generate a Row Name that fits the requirements of the system
	return FText::Format(FTextFormat::FromString(TEXT("Output Row Name:{0}")), FText::FromString(ItemRowName));
}

bool UItemDataEditObject::TryCreateNewItemObject() const
{
	// TODO @gdemers run validation on new item object.
	// i.e do we have remaining space in the bit encoding for the item object type
	// does the name fits our requirements, etc...
	return true;
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
