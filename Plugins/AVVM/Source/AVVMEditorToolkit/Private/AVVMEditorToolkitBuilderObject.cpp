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
#include "AVVMEditorToolkitBuilderObject.h"

#include "AVVMEditorBuilderSubsystem.h"
#include "AVVMEditorToolkitModule.h"
#include "Editor.h"

void UAVVMEditorToolkitBuilderObject::RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core) const
{
	if (OpenTool.IsValid())
	{
		return;
	}

	BIND_NEW_BUTTON_COMMAND(OpenTool, GetOpenCommandName(), GetOpenCommand_Label(), GetOpenCommand_Tooltips());

	if (Core.IsValid())
	{
		Core->GetToolkitCommands()->MapAction(OpenTool,
		                                      FExecuteAction::CreateStatic(&UAVVMEditorToolkitBuilderObjectUtils::SetBuilderObjectActive, this),
		                                      FCanExecuteAction{});
	}
}

void UAVVMEditorToolkitBuilderObjectUtils::SetBuilderObjectActive(const UAVVMEditorToolkitBuilderObject* BuilderObject)
{
	UAVVMEditorBuilderSubsystem::Static_SelectBuilder(GEditor, BuilderObject);
}
