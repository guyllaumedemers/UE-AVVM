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

#include "DataRegistry.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Views/SListView.h"

struct FDataRegistryType;

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitDataVisualizer is a slate context displaying content tied to the active DataRegistryType selection.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitDataVisualizer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitDataVisualizer){};
	SLATE_ATTRIBUTE(TArray<TSharedPtr<const FDataRegistryId>>, ListViewRowEntries)
	SLATE_EVENT(FOnClicked, OnButtonClick_Create)
	SLATE_EVENT(FOnClicked, OnButtonClick_Edit)
	SLATE_EVENT(FOnClicked, OnButtonClick_Delete)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void UpdateDataVisualizer(const FDataRegistryType& NewRegistryType);
	bool GetSelectedItems(TArray<FName>& OutSelectedItems) const;

private:
	TSharedRef<ITableRow> OnGenerateRow(FName RegistryItemName,
	                                    const TSharedRef<STableViewBase>& OwnerTable) const;
	
	void OnSearchTextChanged(const FText& NewText);

	bool OnEnable_ButtonCreate() const;
	bool OnEnable_ButtonEdit() const;
	bool OnEnable_ButtonDelete() const;

	// Holds the persistent reference to the list view
	TSharedPtr<SListView<FName>> ListViewWidget{nullptr};
	TSharedPtr<SScrollBar> ScrollBar{nullptr};
	TStrongObjectPtr<const UDataRegistry> DataRegistry{nullptr};
	TArray<FDataRegistryId> RegistryIds{};
	FName SelectedRegistryType{NAME_None};
	TArray<FName> FilteredEntries{};
	TArray<FName> SourceEntries{};
};
