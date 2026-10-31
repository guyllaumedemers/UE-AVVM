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
#include "SWidgets/AVVMEditorToolkitSplitView.h"

#include "DataRegistrySubsystem.h"
#include "Data/AVVMDataTableRow.h"
#include "SWidgets/AVVMEditorToolkitDataTableRowPreview.h"
#include "Widgets/Input/SButton.h"

void SAVVMEditorToolkitSplitView::Construct(const FArguments& InArgs)
{
	RegistryType_ViewEditDenyList = InArgs._RegistryType_ViewEditDenyList.Get();
	bCanEditSplitView = InArgs._bAreViewEditable.Get();
	RegistryIds = InArgs._DependentRegistryIds.Get();

	SAssignNew(ScrollBar, SScrollBar)
	.AlwaysShowScrollbar(true)
	.Orientation(EOrientation::Orient_Vertical);

	// Name List
	SAssignNew(ListViewWidget, SListView<FName>)
	.ListItemsSource(&FilteredEntries)
	.ExternalScrollbar(ScrollBar)
	.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
	.SelectionMode(ESelectionMode::Single)
	.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("SimpleListView"))
	.OnGenerateRow(this, &SAVVMEditorToolkitSplitView::OnGenerateRow);

	if (!RegistryIds.IsEmpty())
	{
		UpdateSplitView(RegistryIds);
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(NSLOCTEXT("AVVMEditorToolit", "SaveDirtyPackagesTooltip", "Save all modified assets."))
				.OnClicked(InArgs._OnSaveClicked)
				[
					SNew(SImage)
					.DesiredSizeOverride(FVector2d{64.f, 64.f})
					.Image(FAppStyle::Get().GetBrush("MainFrame.SaveAll"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), "LargeText")
				.TransformPolicy(ETextTransformPolicy::ToUpper)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "SplitView_Header", "Step 1: Modify or Save data source, and dependent assets."))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.FillHeight(1.f)
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

FText SAVVMEditorToolkitSplitView::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "SAVVMEditorToolkitSplitView::GetModalMessage_OnClosure", "Are you sure you want to close this window?");
}

FText SAVVMEditorToolkitSplitView::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "SAVVMEditorToolkitSplitView::GetModalTitle_OnClosure", "Warning!");
}

TSharedRef<ITableRow> SAVVMEditorToolkitSplitView::OnGenerateRow(FName InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	const FString AggregatedRegistryId{InItem.ToString()};

	int32 OutSearchIndex{INDEX_NONE};
	AggregatedRegistryId.FindChar(';', OutSearchIndex);

	const FString Left = AggregatedRegistryId.Left(OutSearchIndex);
	const FString Right = AggregatedRegistryId.Right(AggregatedRegistryId.Len() - 1 - OutSearchIndex);

	TSharedPtr<STableRow<FName>> OutListTableRow{};
	SAssignNew(OutListTableRow, STableRow<FName>, OwnerTable)
	[
		SNew(SAVVMEditorToolkitDataTableRowPreview)
		.RegistryType_OnEnableDenyList(RegistryType_ViewEditDenyList)
		.bIsPreviewEditable(bCanEditSplitView)
		.RegistryType(FName(Left))
		.RegistryItemName(FName(Right))
	];

	return OutListTableRow.ToSharedRef();
}

void SAVVMEditorToolkitSplitView::UpdateSplitView(const TArray<FDataRegistryId>& NewRegistryIds)
{
	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return;
	}

	SourceEntries.Empty(NewRegistryIds.Num());
	for (const auto& RegistryId : NewRegistryIds)
	{
		const auto* Row = Subsystem->GetCachedItem<FAVVMDataTableRow>(RegistryId);
		if (ensureAlwaysMsgf(Row != nullptr, TEXT("Failed to retrieved Row type.")))
		{
			FName AggregatedType{FString::Printf(TEXT("%s;%s"), *RegistryId.RegistryType.ToString(), *RegistryId.ItemName.ToString())};
			SourceEntries.Add(MoveTemp(AggregatedType));
		}
	}

	FilteredEntries = SourceEntries;
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RebuildList();
	}
}
