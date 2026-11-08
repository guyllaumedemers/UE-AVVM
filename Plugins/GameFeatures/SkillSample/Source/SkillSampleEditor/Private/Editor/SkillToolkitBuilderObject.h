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

#include "AVVMEditorToolkitBuilderObject.h"

#include "SkillToolkitBuilderObject.generated.h"

class FUICommandInfo;

/**
 *	Class description:
 *	
 *	USkillToolkitBuilderObject is a UObject type used to build editor slate Tool used by
 *	the SkillSample plugin.
 */
UCLASS()
class SKILLSAMPLEEDITOR_API USkillToolkitBuilderObject : public UAVVMEditorToolkitBuilderObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> BuildWidget() const override;
	virtual void RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core) const override;
	virtual TArray<TSharedPtr<FUICommandInfo>> GetUICommands() const override;
	virtual FName GetMenuName() const override;
	virtual FName GetSectionName() const override;
	virtual FName GetSubSectionName() const override;
	virtual FText GetSubSection_Label() const override;
	virtual FText GetSubSection_Tooltips() const override;
};
