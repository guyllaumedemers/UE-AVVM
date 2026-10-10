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

#include "AVVMEditorToolkitDataEditor.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMEditorToolkitDataEditObject is an abstract UObject type that define the internal behaviour
 *	to editing plugin specific data types.
 */
UCLASS(Abstract, NotBlueprintType, NotBlueprintable)
class AVVMEDITORTOOLKIT_API UAVVMEditorToolkitDataEditObject : public UObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> GetDataEditContent() const PURE_VIRTUAL(GetDataEditContent, return nullptr;)
	virtual FText GetModalMessage_OnClosure() const PURE_VIRTUAL(GetModalMessage_OnClosure, return FText::GetEmpty(););
	virtual FText GetModalTitle_OnClosure() const PURE_VIRTUAL(GetModalTitle_OnClosure, return FText::GetEmpty(););
};

/**
 *	Class description:
 *	
 *	EAVVMDataEditorModes is an Enum type to track active mode of an active Window instance.
 */
UENUM(NotBlueprintType)
enum class EAVVMDataEditorModes : uint8
{
	None UMETA(Hidden),
	Create,
	Edit
};

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitDataEditor is a slate context handling data edit modes.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitDataEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitDataEditor){}
	SLATE_ATTRIBUTE(FDataRegistryType, SelectedDataRegistryType)
	SLATE_ATTRIBUTE(FName, SelectedItemName)
	SLATE_ATTRIBUTE(TWeakObjectPtr<const UAVVMEditorToolkitDataEditObject>, DataEditObject)
	SLATE_ATTRIBUTE(EAVVMDataEditorModes, Mode)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedPtr<SWidget> GetDataEditContent() const;
	FText GetModalMessage_OnClosure() const;
	FText GetModalTitle_OnClosure() const;
	
private:
	TWeakObjectPtr<const UAVVMEditorToolkitDataEditObject> DataEditObject{nullptr};
};
