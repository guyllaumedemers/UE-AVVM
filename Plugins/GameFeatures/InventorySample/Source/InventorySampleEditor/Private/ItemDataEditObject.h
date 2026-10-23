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

#include "SWidgets/AVVMEditorToolkitDataEditor.h"

#include "ItemDataEditObject.generated.h"

struct FDataRegistryType;
struct FItemDefinitionDataTableRow;
class UDataRegistry;
class UDataTable;
class UFactory;
class UItemDefinitionDataAsset;
class SAVVMEditorToolkitDataTablePicker;
class SAVVMEditorToolkitDataVisualizer;
class SAVVMEditorToolkitOutputPathPicker;
class SCheckBoxList;

/**
 *	Class description:
 *	
 *	UItemDataEditObject is a context class that define how Item creation/edit should be managed
 *	when using AVVMEditorToolkit.
 */
UCLASS()
class INVENTORYSAMPLEEDITOR_API UItemDataEditObject : public UAVVMEditorToolkitDataEditObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
	                                               const FString& NewDataTablePackagePath,
	                                               const EAVVMDataEditorModes NewMode) override;
	
	FName GetRegistryType_ItemDefinitions() const;
	
	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;

private:
	TSharedPtr<SWidget> GetCreationMode();
	TSharedPtr<SWidget> GetEditMode();
	static const TArray<FName>& GetAssetTypeToDefaultGenerate();
	static const TArray<FName>& GetItemTypes();
	void OnCheckBoxStateChanged(int32 Index);
	void OnOutputAssetPathSelected(const FString& NewPackagePath);
	bool IsEnabled_AssetPathSelectionChanged() const;
	bool IsEnabled_OnItemTypeChanged() const;
	bool OnItemTypeChanged(const FName NewItemType, const int32 NewSelectedIndex);
	void OnDataTableSelectionChanged_ActorIdentifiers(TWeakObjectPtr<UDataTable> NewSelection, ESelectInfo::Type SelectInfo);
	void OnDataTableSelectionChanged_ActorDefinitions(TWeakObjectPtr<UDataTable> NewSelection, ESelectInfo::Type SelectInfo);
	void OnDataTableSelectionChanged_ActorUIDefinitions(TWeakObjectPtr<UDataTable> NewSelection, ESelectInfo::Type SelectInfo);
	bool IsEnabled_GenerateButton() const;
	void OnTextChanged_ItemName(const FText& NewItemName);
	FReply OnButtonClick_GenerateItem();
	
	bool DoesItemObjectNameAlreadyExist(const FString& ItemObjectRowName) const;
	bool TryCreateNewItemObject();
	bool TryCreateDependentAsset(UItemDefinitionDataAsset* ItemDefinitionDataAsset, const FName NewItemType, const FString& ItemObjectRowName, const FString& NewPackagePath);
	int32 TryGenerateNewPhysicalGlobalId(const FName ItemType) const;
	
	FReply EditMode_OnButtonClick_Create();
	FReply EditMode_OnButtonClick_Edit();
	FReply EditMode_OnButtonClick_Delete();
	
	bool OpenDependentAsset(const FName RowName);
	TArray<TWeakObjectPtr<UObject>> GetDependentAssets(const FItemDefinitionDataTableRow& Row) const;

	TSharedPtr<SAVVMEditorToolkitDataTablePicker> DataTablePicker_ActorIdentifiers{nullptr};
	TSharedPtr<SAVVMEditorToolkitDataTablePicker> DataTablePicker_ActorDefinitions{nullptr};
	TSharedPtr<SAVVMEditorToolkitDataTablePicker> DataTablePicker_ActorUIDefinitions{nullptr};
	TSharedPtr<SAVVMEditorToolkitDataVisualizer> EditMode_DataVisualizer{nullptr};
	TSharedPtr<SCheckBoxList> CheckBoxList{nullptr};
	TArray<bool> bCheckBoxStates{};
	TWeakObjectPtr<UDataTable> SelectedDataTable_ItemDefinitions{nullptr};
	TWeakObjectPtr<UDataTable> SelectedDataTable_ActorIdentifiers{nullptr};
	TWeakObjectPtr<UDataTable> SelectedDataTable_ActorDefinitions{nullptr};
	TWeakObjectPtr<UDataTable> SelectedDataTable_ActorUIDefinitions{nullptr};
	FString DataTablePackagePath{};
	FString SelectedPackagePath{};
	FName SelectedItemType{NAME_None};
	FString ItemRowName{};
};

/**
 *	Class description:
 *	
 *	UItemGroupDataEditObject is a context class that define how ItemGroup creation/edit should be managed
 *	when using AVVMEditorToolkit.
 */
UCLASS()
class INVENTORYSAMPLEEDITOR_API UItemGroupDataEditObject : public UAVVMEditorToolkitDataEditObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
	                                               const FString& NewDataTablePackagePath,
	                                               const EAVVMDataEditorModes NewMode) override;
	
	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;
};
