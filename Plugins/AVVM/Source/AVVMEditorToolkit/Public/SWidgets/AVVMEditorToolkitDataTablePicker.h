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
#pragma once

#include "CoreMinimal.h"

#include "AssetThumbnail.h"
#include "DataRegistryId.h"
#include "Framework/SlateDelegates.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Views/SListView.h"

class UDataTable;
using FOnDataTableSelectionChanged = TSlateDelegates<TListTypeTraits<TWeakObjectPtr<UDataTable>>::NullableType>::FOnSelectionChanged;

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitDataTablePicker is a slate context handling DataTable selection.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitDataTablePicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitDataTablePicker){}
	SLATE_EVENT(FOnDataTableSelectionChanged, OnSelectionChanged)
	SLATE_ATTRIBUTE(FDataRegistryType, RegistryType)
	SLATE_ATTRIBUTE(FText, PickerHeaderText)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	bool UpdateDataTableCollection(const FDataRegistryType& NewRegistryType);

private:
	TSharedRef<ITableRow> OnGenerateRow(TWeakObjectPtr<UDataTable> DataTable,
	                                    const TSharedRef<STableViewBase>& OwnerTable) const;
	
	void OnSearchTextChanged(const FText& NewText);
	
	// Holds the persistent reference to the list view
	TSharedPtr<SListView<TWeakObjectPtr<UDataTable>>> ListViewWidget{nullptr};
	TSharedPtr<SScrollBar> ScrollBar{nullptr};
	TArray<TWeakObjectPtr<UDataTable>> FilteredEntries{};
	TArray<TWeakObjectPtr<UDataTable>> SourceEntries{};
	mutable TSharedPtr<FAssetThumbnailPool> ThumbnailPool{nullptr}; 
	mutable TArray<TSharedPtr<FAssetThumbnail>> Thumbnails{};
};
