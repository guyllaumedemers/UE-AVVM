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
#include "SWidgets/AVVMEditorToolkitDataImporter.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/SListView.h"

void SAVVMEditorToolkitDataImporter::Construct(const FArguments& InArgs)
{
	OnDataTypeSelectionChangedDelegate = InArgs._OnDataImporterSourceChanged.Get();
	DataTypes = InArgs._DataTypes.Get();
	
	// Dropdown Button
	SAssignNew(ComboButtonLabelWidget, STextBlock)
	.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
	.Text(this, &SAVVMEditorToolkitDataImporter::OnRowSelectionChanged);

	// Name List
	SAssignNew(ListViewWidget, SListView<FName>)
	.ListItemsSource(&DataTypes)
	.SelectionMode(ESelectionMode::Single)
	.OnMouseButtonClick(this, &SAVVMEditorToolkitDataImporter::OnMouseButtonClick)
	.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("SimpleListView"))
	.OnGenerateRow(this, &SAVVMEditorToolkitDataImporter::OnGenerateRow);

	// Dropdown List content
	SAssignNew(DropdownWidget, SListViewSelectorDropdownMenu<FName>, nullptr, ListViewWidget)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		.Padding(2)
		[
			SNew(SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.MaxHeight(200.0f)
				[
					ListViewWidget.ToSharedRef()
				]
			]
		]
	];

	ChildSlot
	[
		// Combo button that summons the dropdown menu
		SAssignNew(ComboButtonWidget, SComboButton)
		.IsEnabled_Raw(this, &SAVVMEditorToolkitDataImporter::DoesComboBoxHaveElements)
		.ButtonContent()
		[
			ComboButtonLabelWidget.ToSharedRef()
		]
		.MenuContent()
		[
			DropdownWidget.ToSharedRef()
		]
		.IsFocusable(true)
		.ContentPadding(2.0f)
		.OnComboBoxOpened(this, &SAVVMEditorToolkitDataImporter::OnComboBoxOpened)
	];
}

void SAVVMEditorToolkitDataImporter::SelectName(FName NameToSelect,
                                                ESelectInfo::Type SelectionInfo)
{
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->SetSelection(NameToSelect, ESelectInfo::OnMouseClick);
		ListViewWidget->RequestListRefresh();
	}

	OnMouseButtonClick(NameToSelect);
}

void SAVVMEditorToolkitDataImporter::UpdateListViewEntries(TArray<FName>&& NewNameList)
{
	DataTypes = MoveTemp(NewNameList);
	if (!DoesComboBoxHaveElements())
	{
		SelectedDataType = NAME_None;
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RebuildList();
	}
}

void SAVVMEditorToolkitDataImporter::OnMouseButtonClick(FName Item)
{
	SelectedDataType = MoveTemp(Item);
	if (ComboButtonWidget.IsValid())
	{
		ComboButtonWidget->SetIsOpen(false);
	}
	
	OnDataTypeSelectionChangedDelegate.Broadcast(Item);
}

TSharedRef<ITableRow> SAVVMEditorToolkitDataImporter::OnGenerateRow(FName Name,
                                                                    const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<STableRow<FName>> OutListTableRow{};
	SAssignNew(OutListTableRow, STableRow<FName>, OwnerTable)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 1.0f, 20.0f, 1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromName(Name.IsValid() ? Name : FName("Invalid")))
		]
	];

	return OutListTableRow.ToSharedRef();
}

FText SAVVMEditorToolkitDataImporter::OnRowSelectionChanged() const
{
	if (SelectedDataType.IsValid())
	{
		return FText::FromName(SelectedDataType);
	}
	else
	{
		return DoesComboBoxHaveElements() ? NSLOCTEXT("AVVMEditorToolkit", "NameList", "Pick an element...") : NSLOCTEXT("AVVMEditorToolkit", "EmptyList", "Empty...");
	}
}

bool SAVVMEditorToolkitDataImporter::DoesComboBoxHaveElements() const
{
	return DataTypes.Num() > 0;
}

void SAVVMEditorToolkitDataImporter::OnComboBoxOpened()
{
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->SetSelection(SelectedDataType, ESelectInfo::OnKeyPress);
		ListViewWidget->RequestScrollIntoView(SelectedDataType);
	}
}
