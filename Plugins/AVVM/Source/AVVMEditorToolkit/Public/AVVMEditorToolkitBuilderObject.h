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

/**
 *	Class description:
 *	
 *	FAVVMBuilderInfo is a context object that encapsulate information required
 *	by Slate to extend specific Menus, SubMenus, etc...
 */
struct AVVMEDITORTOOLKIT_API FAVVMBuilderInfo
{
	FName SectionName = NAME_None;
	FName SubSectionName = NAME_None;
	FText SubSection_Label = FText::GetEmpty();
	FText SubSection_Tooltips = FText::GetEmpty();
	FSlateIcon SubSection_Icon = FSlateIcon();
	TArray<TSharedPtr<FUICommandInfo>> Commands;
};

/**
 *	Class description:
 *	
 *	UAVVMEditorToolkitBuilderObject is a UObject type that define the layout constraint of a plugin Toolkit. Each module/plugin can derive from this object
 *	and provide impl details specific to their feature definition. Doing so allow flexibility, and customization, of the AVVMEditorToolkit context window
 *	without requiring modification of the underline system.
 */
UCLASS(BlueprintType)
class AVVMEDITORTOOLKIT_API UAVVMEditorToolkitBuilderObject : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core) PURE_VIRTUAL(RegisterCommands, return;);
	void GetBuilderInfo(FAVVMBuilderInfo& OutBuilderInfo) const;

protected:
	virtual TArray<TSharedPtr<FUICommandInfo>> GetUICommands() const PURE_VIRTUAL(GetUICommands, return {};);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SectionName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SubSectionName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SubSection_Label = FText::GetEmpty();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SubSection_Tooltips = FText::GetEmpty();
};
