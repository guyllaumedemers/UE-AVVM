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
#include "CosmeticToolkitBuilderObject.h"

#include "CosmeticToolkitWindow.h"

TSharedPtr<SWidget> UCosmeticToolkitBuilderObject::BuildWidget() const
{
	if (!BuilderContextWidget.IsValid())
	{
		BuilderContextWidget = SNew(SCosmeticToolkitWindow);
	}

	return BuilderContextWidget;
}

TArray<TSharedPtr<FUICommandInfo>> UCosmeticToolkitBuilderObject::GetUICommands() const
{
	return
	{
			OpenTool
	};
}

FName UCosmeticToolkitBuilderObject::GetMenuName() const
{
	return TEXT("Menu_CosmeticSample");
}

FName UCosmeticToolkitBuilderObject::GetSectionName() const
{
	return TEXT("Section_CosmeticSample");
}

FName UCosmeticToolkitBuilderObject::GetSubSectionName() const
{
	return TEXT("SubSection_CosmeticSample");
}

FText UCosmeticToolkitBuilderObject::GetSubSection_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UCosmeticToolkitBuilderObject::GetSubSection_Label", "CosmeticSampleEditor");
}

FText UCosmeticToolkitBuilderObject::GetSubSection_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UCosmeticToolkitBuilderObject::GetSubSection_Tooltips", "Central hub in which designers can create/edit/delete elements from the cosmetic system.");
}

FName UCosmeticToolkitBuilderObject::GetOpenCommandName() const
{
	return TEXT("Open_CosmeticSample");
}

FText UCosmeticToolkitBuilderObject::GetOpenCommand_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UCosmeticToolkitBuilderObject::GetOpenCommand_Label", "Open CosmeticSample");
}

FText UCosmeticToolkitBuilderObject::GetOpenCommand_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UCosmeticToolkitBuilderObject::GetOpenCommand_Tooltips", "Allow opening the context tooling required for data editing asset specific to the cosmetic system.");
}
