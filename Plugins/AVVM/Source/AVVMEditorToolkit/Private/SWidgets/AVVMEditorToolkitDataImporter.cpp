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
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/SListView.h"

void SAVVMEditorToolkitDataImporter::Construct(const FArguments& InArgs)
{
	OnDataRegistryTypeSelectionChangedDelegate = InArgs._OnDataImporterSourceChanged;
	DataRegistryTypes = InArgs._DataRegistryTypes.Get();
	
	// Dropdown Button
	SAssignNew(ComboButtonLabelWidget, STextBlock)
	.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
	.Text(this, &SAVVMEditorToolkitDataImporter::OnRowSelectionChanged);

	// Name List
	SAssignNew(ListViewWidget, SListView<FName>)
	.ListItemsSource(&DataRegistryTypes)
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
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Create"))
					.TextStyle(FAppStyle::Get(), "DialogButtonText")
					.HAlign(HAlign_Center)
					.OnClicked(this, &SAVVMEditorToolkitDataImporter::OnButtonClick_Create)
					.IsEnabled(this, &SAVVMEditorToolkitDataImporter::OnEnable_ButtonCreate)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Edit"))
					.TextStyle(FAppStyle::Get(), "DialogButtonText")
					.HAlign(HAlign_Center)
					.OnClicked(this, &SAVVMEditorToolkitDataImporter::OnButtonClick_Edit)
					.IsEnabled(this, &SAVVMEditorToolkitDataImporter::OnEnable_ButtonEdit)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Delete"))
					.TextStyle(FAppStyle::Get(), "DialogButtonText")
					.HAlign(HAlign_Center)
					.OnClicked(this, &SAVVMEditorToolkitDataImporter::OnButtonClick_Delete)
					.IsEnabled(this, &SAVVMEditorToolkitDataImporter::OnEnable_ButtonDelete)
				]
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				// Combo button that summons the dropdown menu
				SAssignNew(ComboButtonWidget, SComboButton)
				.IsEnabled(this, &SAVVMEditorToolkitDataImporter::DoesComboBoxHaveElements)
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
			]
		]
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
	DataRegistryTypes = MoveTemp(NewNameList);
	if (!DoesComboBoxHaveElements())
	{
		DataRegistryTypeSelected = NAME_None;
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RebuildList();
	}
}

void SAVVMEditorToolkitDataImporter::OnMouseButtonClick(FName Item)
{
	DataRegistryTypeSelected = MoveTemp(Item);
	if (ComboButtonWidget.IsValid())
	{
		ComboButtonWidget->SetIsOpen(false);
	}

	if (ensureAlwaysMsgf(OnDataRegistryTypeSelectionChangedDelegate.IsBound(),
	                     TEXT("Unbound delegate.")))
	{
		bDoesDataRegistryHaveRows = OnDataRegistryTypeSelectionChangedDelegate.Execute(Item);
	}
}

TSharedRef<ITableRow> SAVVMEditorToolkitDataImporter::OnGenerateRow(FName Name,
                                                                    const TSharedRef<STableViewBase>& OwnerTable) const
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
	if (DataRegistryTypeSelected.IsValid())
	{
		return FText::FromName(DataRegistryTypeSelected);
	}
	else
	{
		return DoesComboBoxHaveElements() ? NSLOCTEXT("AVVMEditorToolkit", "NameList", "Pick an element...") : NSLOCTEXT("AVVMEditorToolkit", "EmptyList", "Empty...");
	}
}

bool SAVVMEditorToolkitDataImporter::DoesComboBoxHaveElements() const
{
	return DataRegistryTypes.Num() > 0;
}

void SAVVMEditorToolkitDataImporter::OnComboBoxOpened()
{
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->SetSelection(DataRegistryTypeSelected, ESelectInfo::OnKeyPress);
		ListViewWidget->RequestScrollIntoView(DataRegistryTypeSelected);
	}
}

FReply SAVVMEditorToolkitDataImporter::OnButtonClick_Create()
{
	return FReply::Handled();
}

FReply SAVVMEditorToolkitDataImporter::OnButtonClick_Edit()
{
	return FReply::Handled();
}

FReply SAVVMEditorToolkitDataImporter::OnButtonClick_Delete()
{
	return FReply::Handled();
}

bool SAVVMEditorToolkitDataImporter::OnEnable_ButtonCreate() const
{
	return (false == DataRegistryTypeSelected.IsNone());
}

bool SAVVMEditorToolkitDataImporter::OnEnable_ButtonEdit() const
{
	return (false == DataRegistryTypeSelected.IsNone()) && bDoesDataRegistryHaveRows;
}

bool SAVVMEditorToolkitDataImporter::OnEnable_ButtonDelete() const
{
	return OnEnable_ButtonEdit();
}
