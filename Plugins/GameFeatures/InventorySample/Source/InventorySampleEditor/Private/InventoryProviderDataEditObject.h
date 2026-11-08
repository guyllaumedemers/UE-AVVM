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

#include "GameplayTagContainer.h"
#include "SWidgets/AVVMEditorToolkitDataEditor.h"

#include "InventoryProviderDataEditObject.generated.h"

class SCheckBoxList;
class UAVVMTagArrayBuilder;
class UItemObject;

/**
 *	Class description:
 *	
 *	UItemArrayBuilder is a wrapper type for exposing Array collection of items to the editor. Using UObject derive type,
 *	we can use the FPropertyEditor, and the underline SExpandableArea + FDetailArrayBuilder to handle collection change actions such as Add/Remove.
 */
UCLASS(NotBlueprintable)
class INVENTORYSAMPLEEDITOR_API UItemArrayBuilder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftClassPtr<UItemObject>> DefaultItems{};
};

/**
 *	Class description:
 *	
 *	UFtueInventoryProviderDataEditObject is a context class that define how Ftue InventoryProvider creation/edit should be managed
 *	when using AVVMEditorToolkit.
 */
UCLASS()
class INVENTORYSAMPLEEDITOR_API UFtueInventoryProviderDataEditObject : public UAVVMEditorToolkitDataEditObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> GetDataEditContent(const FAVVMEditContextArgs& Params) override;

	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;

private:
	TSharedPtr<SWidget> GetCreationMode();
	TSharedPtr<SWidget> GetEditMode();
	// BEGIN - @gdemers creation mode api
	bool IsEnabled_OnActorClassChanged() const;
	bool IsEnabled_OnSlotTagRequirementsChanged() const;
	bool IsEnabled_GenerateButton() const;
	const UClass* OnGetClass() const;
	void OnSetClass(const UClass* NewSelectedClass);
	ECheckBoxState GetOnCheckboxState() const;
	void OnCheckBoxStateChanged(ECheckBoxState NewState);
	void OnTagComboChanged(const FGameplayTagContainer& TagContainer);
	FGameplayTagContainer GetTagContainer() const;
	void OnTagCleared();
	FReply OnButtonClick_GenerateItem();
	// END

	FGameplayTagContainer LoadoutSlots{FGameplayTagContainer::EmptyContainer};
	TStrongObjectPtr<UItemArrayBuilder> DefaultInventory{nullptr};
	TWeakObjectPtr<const UClass> ActorClass{nullptr};
	ECheckBoxState CheckboxState{ECheckBoxState::Unchecked};
};

/**
 *	Class description:
 *	
 *	UStubDataInventoryDependencyGraphEditObject is a context class that define how Stub Data (Backend) Inventory Dependency Graph creation/edit should be managed
 *	when using AVVMEditorToolkit.
 */
UCLASS()
class INVENTORYSAMPLEEDITOR_API UStubDataInventoryDependencyGraphEditObject : public UAVVMEditorToolkitDataEditObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> GetDataEditContent(const FAVVMEditContextArgs& Params) override;

	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;
};
