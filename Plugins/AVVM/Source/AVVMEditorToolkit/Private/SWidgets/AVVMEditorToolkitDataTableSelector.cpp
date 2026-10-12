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
#include "SWidgets/AVVMEditorToolkitDataTableSelector.h"

#include "AVVMEditorToolkitUtils.h"
#include "DataRegistrySource_DataTable.h"
#include "Engine/DataTable.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/Views/SListView.h"

void SAVVMEditorToolkitDataTableSelector::Construct(const FArguments& InArgs)
{
	SAssignNew(ScrollBar, SScrollBar)
	.AlwaysShowScrollbar(true)
	.Orientation(EOrientation::Orient_Vertical);

	SAssignNew(ListViewWidget, SListView<TWeakObjectPtr<UDataTable>>)
	.ListItemsSource(&FilteredEntries)
	.ExternalScrollbar(ScrollBar)
	.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
	.SelectionMode(ESelectionMode::Single)
	.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("SimpleListView"))
	.OnGenerateRow(this, &SAVVMEditorToolkitDataTableSelector::OnGenerateRow);
	
	UpdateDataTableCollection(InArgs._RegistryType.Get());

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SSearchBox)
				.HintText(FText::FromString(TEXT("Search elements...")))
				.OnTextChanged(this, &SAVVMEditorToolkitDataTableSelector::OnSearchTextChanged)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0.f)
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
		]
	];
}

void SAVVMEditorToolkitDataTableSelector::UpdateDataTableCollection(const FDataRegistryType& NewRegistryType)
{
	if (!ensureAlwaysMsgf(NewRegistryType.IsValid(),
	                      TEXT("Invalid RegistryType.")))
	{
		return;
	}

	const TArray<const UDataRegistrySource_DataTable*> OutResult = UAVVMEditorToolkitUtils::GetDataTables(NewRegistryType);
	if (OutResult.IsEmpty())
	{
		return;
	}

	SourceEntries.Empty(OutResult.Num());
	for (const auto* DataTableSource : OutResult)
	{
		if (IsValid(DataTableSource))
		{
			SourceEntries.Add(DataTableSource->SourceTable.LoadSynchronous());
		}
	}

	FilteredEntries = SourceEntries;
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RebuildList();
	}
}

TSharedRef<ITableRow> SAVVMEditorToolkitDataTableSelector::OnGenerateRow(TWeakObjectPtr<UDataTable> DataTable,
                                                                         const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<STableRow<TWeakObjectPtr<UDataTable>>> OutListTableRow{};
	SAssignNew(OutListTableRow, STableRow<TWeakObjectPtr<UDataTable>>, OwnerTable)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 1.0f, 20.0f, 1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(DataTable.IsValid() ? DataTable->GetName() : TEXT("Invalid")))
		]
	];

	return OutListTableRow.ToSharedRef();
}

void SAVVMEditorToolkitDataTableSelector::OnSearchTextChanged(const FText& NewText)
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
			if (Item.IsValid() && Item->GetName().Contains(FilterString, ESearchCase::IgnoreCase))
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
