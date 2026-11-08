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

#include "UObject/Object.h"

#include "AVVMEditorToolkitBuilderObject.generated.h"

class FAVVMEditorToolkit_Core;
class FUICommandInfo;
class SWidget;

/**
 *	Class description:
 *	
 *	UAVVMEditorToolkitBuilderObject is a UObject type that define the layout constraint of a plugin Toolkit. Each module/plugin can derive from this object
 *	and provide impl details specific to their feature definition. Doing so allow flexibility, and customization, of the AVVMEditorToolkit context window
 *	without requiring modification of the underline system.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class AVVMEDITORTOOLKIT_API UAVVMEditorToolkitBuilderObject : public UObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> BuildWidget() const PURE_VIRTUAL(BuildWidget, return BuilderContextWidget;);
	virtual void RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core) const PURE_VIRTUAL(RegisterCommands, return;);
	virtual TArray<TSharedPtr<FUICommandInfo>> GetUICommands() const PURE_VIRTUAL(GetUICommands, return {};);
	virtual FName GetMenuName() const PURE_VIRTUAL(GetMenuName, return NAME_None;)
	virtual FName GetSectionName() const PURE_VIRTUAL(GetSectionName, return NAME_None;)
	virtual FName GetSubSectionName() const PURE_VIRTUAL(GetSubSectionName, return NAME_None;)
	virtual FText GetSubSection_Label() const PURE_VIRTUAL(GetSubSection_Label, return FText::GetEmpty();)
	virtual FText GetSubSection_Tooltips() const PURE_VIRTUAL(GetSubSection_Tooltips, return FText::GetEmpty();)
	void SetActiveSelf() const;

protected:
	/** Pointer to the widget that houses the level editor's mode context window */
	mutable TSharedPtr<SWidget> BuilderContextWidget = nullptr;
	// @gdemers UICommand that execute Opening the window context
	// for the sample editor tool impl.
	mutable TSharedPtr<FUICommandInfo> OpenTool = nullptr;
};
