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

#include "AVVMDoesWidgetProvideModalMessaging.h"
#include "DataRegistryId.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitSplitView is a slate context that handle displaying dependent assets
 *	into a singular view context.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitSplitView : public SCompoundWidget,
                                                          public IAVVMDoesWidgetProvideModalMessaging
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitSplitView){}
	SLATE_ATTRIBUTE(TArray<FDataRegistryId>, DependentRegistryIds)
	SLATE_ATTRIBUTE(TArray<FName>, RegistryType_ViewEditDenyList)
	SLATE_EVENT(FOnClicked, OnSaveClicked)
	SLATE_ATTRIBUTE(bool, bAreViewEditable)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;

private:
	TSharedRef<ITableRow> OnGenerateRow(FName InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	void UpdateSplitView(const TArray<FDataRegistryId>& NewRegistryIds);
	
	TSharedPtr<SListView<FName>> ListViewWidget{nullptr};
	TSharedPtr<SScrollBar> ScrollBar{nullptr};
	TArray<FDataRegistryId> RegistryIds{};
	TArray<FName> FilteredEntries{};
	TArray<FName> SourceEntries{};
	TArray<FName> RegistryType_ViewEditDenyList{};
	bool bCanEditSplitView{false};
};
