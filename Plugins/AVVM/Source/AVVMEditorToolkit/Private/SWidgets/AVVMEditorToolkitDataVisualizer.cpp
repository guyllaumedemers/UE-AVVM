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
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"

#include "DataRegistrySubsystem.h"
#include "Components/VerticalBox.h"
#include "Data/AVVMDataTableRow.h"
#include "SWidgets/AVVMEditorToolkitDataTableRowPreview.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"

void SAVVMEditorToolkitDataVisualizer::Construct(const FArguments& InArgs)
{
	SAssignNew(ScrollBar, SScrollBar)
	.AlwaysShowScrollbar(true)
	.Orientation(EOrientation::Orient_Vertical);

	SAssignNew(ListViewWidget, SListView<FName>)
	.ListItemsSource(&FilteredEntries)
	.ExternalScrollbar(ScrollBar)
	.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
	.SelectionMode(ESelectionMode::SingleToggle)
	.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("SimpleListView"))
	.OnGenerateRow(this, &SAVVMEditorToolkitDataVisualizer::OnGenerateRow);

	if (InArgs._DataRegistryType.Get().IsValid())
	{
		UpdateDataVisualizer(InArgs._DataRegistryType.Get());
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(12.f)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Create"))
					.TextStyle(FAppStyle::Get(), "DialogButtonText")
					.HAlign(HAlign_Center)
					.OnClicked(InArgs._OnButtonClick_Create)
					.IsEnabled(this, &SAVVMEditorToolkitDataVisualizer::OnEnable_ButtonCreate)
				]
				+ SHorizontalBox::Slot()
				.Padding(12.f)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Edit"))
					.TextStyle(FAppStyle::Get(), "DialogButtonText")
					.HAlign(HAlign_Center)
					.OnClicked(InArgs._OnButtonClick_Edit)
					.IsEnabled(this, &SAVVMEditorToolkitDataVisualizer::OnEnable_ButtonEdit)
				]
				+ SHorizontalBox::Slot()
				.Padding(12.f)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Delete"))
					.TextStyle(FAppStyle::Get(), "DialogButtonText")
					.HAlign(HAlign_Center)
					.OnClicked(InArgs._OnButtonClick_Delete)
					.IsEnabled(this, &SAVVMEditorToolkitDataVisualizer::OnEnable_ButtonDelete)
				]
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SSearchBox)
				.HintText(FText::FromString(TEXT("Search elements...")))
				.OnTextChanged(this, &SAVVMEditorToolkitDataVisualizer::OnSearchTextChanged)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					ListViewWidget.ToSharedRef()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(FOptionalSize(16))
					[
						ScrollBar.ToSharedRef()
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(12.f)
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.AutoWidth()
				[
					SNew(SCheckBox)
				]
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("AVVMEditorToolkit", "SAVVMEditorToolkitDataImporter", "Open Dependent Assets"))
				]
			]
		]
	];
}

bool SAVVMEditorToolkitDataVisualizer::GetSelectedItems(TArray<FName>& OutSelectedItems) const
{
	if (ListViewWidget.IsValid())
	{
		return (ListViewWidget->GetSelectedItems(OutSelectedItems) > 0);
	}
	else
	{
		return false;
	}
}

TSharedRef<ITableRow> SAVVMEditorToolkitDataVisualizer::OnGenerateRow(FName RegistryItemName,
                                                                      const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<STableRow<FName>> OutListTableRow{};
	SAssignNew(OutListTableRow, STableRow<FName>, OwnerTable)
	[
		SNew(SAVVMEditorToolkitDataTableRowPreview)
		.RegistryType(SelectedRegistryType)
		.RegistryItemName(RegistryItemName)
	];

	return OutListTableRow.ToSharedRef();
}

void SAVVMEditorToolkitDataVisualizer::UpdateDataVisualizer(const FDataRegistryType& NewRegistryType)
{
	if (!ensureAlwaysMsgf(NewRegistryType.IsValid(),
	                      TEXT("Invalid RegistryType.")))
	{
		return;
	}

	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return;
	}

	DataRegistry = TStrongObjectPtr(Subsystem->GetRegistryForType(NewRegistryType));
	if (!DataRegistry.IsValid())
	{
		return;
	}

	SelectedRegistryType = DataRegistry->GetRegistryType();
	DataRegistry->GetPossibleRegistryIds(RegistryIds);
	if (RegistryIds.IsEmpty() || !SelectedRegistryType.IsValid())
	{
		return;
	}

	SourceEntries.Empty(RegistryIds.Num());
	for (const auto& RegistryId : RegistryIds)
	{
		SourceEntries.Add(RegistryId.ItemName);
	}

	// @gdemers copy dont move
	FilteredEntries = SourceEntries;
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RebuildList();
	}
}

void SAVVMEditorToolkitDataVisualizer::OnSearchTextChanged(const FText& NewText)
{
	FilteredEntries.Reset(SourceEntries.Num());

	const FString FilterString = NewText.ToString().TrimStartAndEnd();
	if (FilterString.IsEmpty())
	{
		FilteredEntries = SourceEntries;
	}
	else
	{
		for (const auto& Item : SourceEntries)
		{
			if (Item.IsValid() && Item.ToString().Contains(FilterString, ESearchCase::IgnoreCase))
			{
				FilteredEntries.Add(Item);
			}
		}
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}

bool SAVVMEditorToolkitDataVisualizer::OnEnable_ButtonCreate() const
{
	return (false == SelectedRegistryType.IsNone());
}

bool SAVVMEditorToolkitDataVisualizer::OnEnable_ButtonEdit() const
{
	return (false == SelectedRegistryType.IsNone()) && (SourceEntries.Num() > 0) && (ListViewWidget.IsValid() && (ListViewWidget->GetNumItemsSelected() > 0));
}

bool SAVVMEditorToolkitDataVisualizer::OnEnable_ButtonDelete() const
{
	return OnEnable_ButtonEdit();
}
