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
#include "Editor/SkillToolkitBuilderObject.h"

#include "AVVMEditorToolkitModule.h"
#include "SkillToolkitWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 *	@gdemers Private namespace for reusable api specific to the AVVM DataTableEditor toolkit.
 *	
 *	Note : Inline namespace expose the internal content to the parent scope, allowing me
 *	to declare a name for accessing the nested content without affecting linkage properties.
 */
inline namespace NS_SkillDataTableEditor
{
	// @gdemers anonymous namespace provide support for implicit internal linkage (no static required),
	// and expose to the parent scope.
	namespace
	{
		void OpenTool(const USkillToolkitBuilderObject* BuilderContextObject)
		{
			if (IsValid(BuilderContextObject))
			{
				UE_LOG(LogTemp, Log, TEXT("NS_AVVMDataTableEditor::OpenTool"));
				BuilderContextObject->SetActiveSelf();
			}
		}
	}
}

TSharedPtr<SWidget> USkillToolkitBuilderObject::BuildWidget() const
{
	if (!BuilderContextWidget.IsValid())
	{
		BuilderContextWidget = SNew(SSkillToolkitWindow);
	}

	return BuilderContextWidget;
}

void USkillToolkitBuilderObject::RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core) const
{
	if (OpenTool.IsValid())
	{
		return;
	}

	BIND_NEW_BUTTON_COMMAND(OpenTool,
	                        FName("SkillSample_OpenTool"),
	                        NSLOCTEXT("AVVMEditorToolkit", "Open SkillSample", "Open SkillSample"),
	                        NSLOCTEXT("AVVMEditorToolkit", "Lorem Ipsum", "Lorem Ipsum"));

	if (Core.IsValid())
	{
		Core->GetToolkitCommands()->MapAction(OpenTool,
		                                      FExecuteAction::CreateStatic(&NS_SkillDataTableEditor::OpenTool, this),
		                                      FCanExecuteAction{});
	}
}

TArray<TSharedPtr<FUICommandInfo>> USkillToolkitBuilderObject::GetUICommands() const
{
	return
	{
			OpenTool
	};
}

FName USkillToolkitBuilderObject::GetMenuName() const
{
	return TEXT("Menu_SkillSample");
}

FName USkillToolkitBuilderObject::GetSectionName() const
{
	return TEXT("Section_SkillSample");
}

FName USkillToolkitBuilderObject::GetSubSectionName() const
{
	return TEXT("SubSection_SkillSample");
}

FText USkillToolkitBuilderObject::GetSubSection_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "USkillToolkitBuilderObject::GetSubSection_Label", "SkillSampleEditor");
}

FText USkillToolkitBuilderObject::GetSubSection_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "USkillToolkitBuilderObject::GetSubSection_Tooltips", "Central hub in which designers can create/edit/delete elements from the skill system.");
}
