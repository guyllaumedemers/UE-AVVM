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
#include "SWidgets/AVVMEditorToolkitDataTablePicker.h"

#include "AVVMEditorToolkitUtils.h"
#include "DataRegistrySource_DataTable.h"
#include "Engine/DataTable.h"
#include "Factories/Factory.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/Views/SListView.h"

void SAVVMEditorToolkitDataTablePicker::Construct(const FArguments& InArgs)
{
	SAssignNew(ScrollBar, SScrollBar)
	.AlwaysShowScrollbar(true)
	.AlwaysShowScrollbarTrack(true)
	.Orientation(EOrientation::Orient_Vertical);

	SAssignNew(ListViewWidget, SListView<TWeakObjectPtr<UDataTable>>)
	.ListItemsSource(&FilteredEntries)
	.ExternalScrollbar(ScrollBar)
	.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
	.SelectionMode(ESelectionMode::SingleToggle)
	.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("SimpleListView"))
	.OnGenerateRow(this, &SAVVMEditorToolkitDataTablePicker::OnGenerateRow)
	.OnSelectionChanged(InArgs._OnSelectionChanged);

	ThumbnailPool = MakeShareable(new FAssetThumbnailPool(32));

	if (InArgs._RegistryType.Get().IsValid())
	{
		UpdateDataTableCollection(InArgs._RegistryType.Get());
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
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.Font(FAppStyle::GetFontStyle("NormalFontBold"))
				.Text(InArgs._PickerHeaderText)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SSearchBox)
				.HintText(FText::FromString(TEXT("Search elements...")))
				.OnTextChanged(this, &SAVVMEditorToolkitDataTablePicker::OnSearchTextChanged)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
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

bool SAVVMEditorToolkitDataTablePicker::UpdateDataTableCollection(const FDataRegistryType& NewRegistryType)
{
	if (!ensureAlwaysMsgf(NewRegistryType.IsValid(),
	                      TEXT("Invalid RegistryType.")))
	{
		return false;
	}

	const TArray<const UDataRegistrySource_DataTable*> OutResult = UAVVMEditorToolkitUtils::GetDataTables(NewRegistryType);
	if (OutResult.IsEmpty())
	{
		return false;
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

	return (SourceEntries.Num() > 0);
}

TSharedRef<ITableRow> SAVVMEditorToolkitDataTablePicker::OnGenerateRow(TWeakObjectPtr<UDataTable> DataTable,
                                                                       const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedPtr<FAssetThumbnail> NewThumbnail = MakeShareable(new FAssetThumbnail(DataTable.Get(), 64, 64, ThumbnailPool));

	TSharedPtr<STableRow<TWeakObjectPtr<UDataTable>>> OutListTableRow{};
	SAssignNew(OutListTableRow, STableRow<TWeakObjectPtr<UDataTable>>, OwnerTable)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 1.0f, 20.0f, 1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.AutoWidth()
				[
					NewThumbnail.IsValid() ? NewThumbnail->MakeThumbnailWidget() : SNullWidget::NullWidget
				]
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.AutoWidth()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Left)
						.TextStyle(FAppStyle::Get(), "LargeText")
						.Text(FText::FromString(DataTable.IsValid() ? DataTable->GetName() : TEXT("Error")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Left)
						.TextStyle(FAppStyle::Get(), "SmallText")
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						.Text(FText::FromString(DataTable.IsValid() ? DataTable->GetPathName() : TEXT("No Path")))
					]
				]
			]
		]
	];

	Thumbnails.Add(MoveTemp(NewThumbnail));
	return OutListTableRow.ToSharedRef();
}

void SAVVMEditorToolkitDataTablePicker::OnSearchTextChanged(const FText& NewText)
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
