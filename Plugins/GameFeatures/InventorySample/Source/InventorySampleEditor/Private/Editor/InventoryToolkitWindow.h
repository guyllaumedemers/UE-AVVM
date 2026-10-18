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

#include "DataRegistryId.h"
#include "Widgets/SCompoundWidget.h"

struct FSlateBrush;
class SAVVMEditorToolkitDataTablePicker;
class SCheckBoxList;
class UAVVMEditorToolkitDataEditObject;
class UDataRegistry;
class UDataTable;
class UDataTableFactory;
class UFactory;

/**
 *	Class description:
 *	
 *	SInventoryToolkitWindow is a slate context for handling data table edits
 *	for the inventory system.
 */
class INVENTORYSAMPLEEDITOR_API SInventoryToolkitWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInventoryToolkitWindow){};
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FText PluginTitle() const;
	FText PluginDescription() const;
	FText PluginVersion() const;
	FSlateBrush* PluginIcon() const;

	bool OnDataRegistrySelectionChanged(const FName SelectedRegistryType, const int32 NewSelectedIndex);
	void OnCheckBoxStateChanged(int32 Index);
	void OnOutputAssetPathSelected(const FString& NewPath);
	void OnDataTableSelectionChanged(TWeakObjectPtr<UDataTable> NewSelection, ESelectInfo::Type SelectInfo);
	bool IsEnabled_AssetPathSelectionChanged() const;
	bool IsEnabled_DataTableAvailableChanged() const;
	bool IsEnabled_DataRegistrySelectionChanged() const;
	TArray<FName> GetAssetTypeToDefaultGenerate() const;
	TArray<FName> GetInventoryDataRegistryTypes() const;
	TArray<UScriptStruct*> GetInventoryRowTypes() const;
	FDataRegistryType GetDataRegistryType() const;
	FReply OnButtonClick_OpenWindow();
	FText OnTextChanged_CreateOrEdit() const;

	void CreateDataTable();
	void EditDataTable();
	void OpenWindow(const FName RegistryType, const bool bShouldOpenInEditMode);
	void OnWindowClosedOverride(const TSharedRef<SWindow>& PendingCloseWindow) const;
	void OnWindowClosed(const TSharedRef<SWindow>& PendingCloseWindow);
	
	bool CreateNewDataRegistry(const int32 NewSelectedRegistryTypeIndex) const;
	
	const UAVVMEditorToolkitDataEditObject* GetDataEditObject(const FName RegistryType) const;
	
	TArray<TSharedPtr<SWindow>> FloatingWindows{};
	TSharedPtr<SAVVMEditorToolkitDataTablePicker> DataTablePicker{nullptr};
	TSharedPtr<SCheckBoxList> CheckBoxList{nullptr};
	TArray<bool> bCheckBoxStates{};
	TWeakObjectPtr<UDataRegistry> SelectedDataRegistry{nullptr};
	FDataRegistryType SelectedDataRegistryType{};
	FString SelectedAssetPath{};
	int32 SelectedRegistryTypeIndex{INDEX_NONE};
	TWeakObjectPtr<UDataTable> SelectedDataTable{nullptr};
	bool bHasDataTableAvailable{false};
	bool bShouldEditDataTable{false};
};
