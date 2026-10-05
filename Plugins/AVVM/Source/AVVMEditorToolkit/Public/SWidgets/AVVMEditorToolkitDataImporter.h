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

#include "SListViewSelectorDropdownMenu.h"
#include "Widgets/SCompoundWidget.h"

class SComboButton;
class STextBlock;

DECLARE_DELEGATE_RetVal_OneParam(bool, SOnDataImporterSourceChangedDelegate, FName);

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitDataImporter is a slate context handling DataRegistryType selection.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitDataImporter : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitDataImporter){};
	SLATE_EVENT(SOnDataImporterSourceChangedDelegate, OnDataImporterSourceChanged)
	SLATE_ATTRIBUTE(TArray<FName>, DataRegistryTypes)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void SelectName(FName NameToSelect, ESelectInfo::Type SelectionInfo = ESelectInfo::Direct);
	void UpdateListViewEntries(TArray<FName>&& NewNameList);
	
private:
	void OnMouseButtonClick(FName Item);
	TSharedRef<ITableRow> OnGenerateRow(FName Name, const TSharedRef<STableViewBase>& OwnerTable) const;
	FText OnRowSelectionChanged() const;
	bool DoesComboBoxHaveElements() const;
	void OnComboBoxOpened();
	
	FReply OnButtonClick_Create();
	FReply OnButtonClick_Edit();
	FReply OnButtonClick_Delete();

	bool OnEnable_ButtonCreate() const;
	bool OnEnable_ButtonEdit() const;
	bool OnEnable_ButtonDelete() const;
	
	TSharedPtr<SListViewSelectorDropdownMenu<FName>> DropdownWidget{nullptr};
	TSharedPtr<SListView<FName>> ListViewWidget{nullptr};
	TSharedPtr<STextBlock> ComboButtonLabelWidget{nullptr};
	TSharedPtr<SComboButton> ComboButtonWidget{nullptr};
	
	SOnDataImporterSourceChangedDelegate OnDataRegistryTypeSelectionChangedDelegate{}; 
	bool bDoesDataRegistryHaveRows{false};
	FName DataRegistryTypeSelected{NAME_None};
	TArray<FName> DataRegistryTypes{};
};
