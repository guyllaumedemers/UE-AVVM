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

class IDetailsView;

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
	virtual TSharedPtr<SWidget> GetDataEditContent(const EAVVMDataEditorModes CurrentMode) override;
	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;

private:
	TArray<FName> GetItemTypes() const;
	FReply OnButtonClick_GenerateElementId() const;
	bool OnDataCategoryTypeChanged(FName NewItemCategoryType);
	void OnTextFieldChanged(const FText& NewText);
	void OnAssetPathSelected(const FString& NewPath);
	bool OnEnable_ButtonGenerateElementId() const;
	bool OnEnable_EditableText() const;
	bool OnEnable_BlueprintItemObjectDetailView() const;
	bool OnEnable_TableRowItemObjectDetailView() const;
	
	FText GetAssetOutputPath() const;
	FText GetItemObjectNameGenerated() const;
	
	bool TryCreateNewItemObject() const;
	
	FString MakeNewItemObjectName() const;
	bool DoesItemObjectNameAlreadyExist(const FString& NewValue) const;
	
	TSharedPtr<IDetailsView> BlueprintItemObjectPropertyView{nullptr};
	TSharedPtr<IDetailsView> TableRowItemObjectPropertyView{nullptr};
	FName SelectedItemCategoryType{NAME_None};
	FString ItemRowName{};
	FString AssetOutputPath{};
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
	virtual TSharedPtr<SWidget> GetDataEditContent(const EAVVMDataEditorModes CurrentMode) override;
	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;
};
