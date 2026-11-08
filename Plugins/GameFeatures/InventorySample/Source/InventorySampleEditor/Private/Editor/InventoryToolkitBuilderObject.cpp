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
#include "InventoryToolkitBuilderObject.h"

#include "AVVMEditorToolkitModule.h"
#include "InventoryToolkitWindow.h"
#include "Framework/Commands/UICommandInfo.h"

TSharedPtr<SWidget> UInventoryToolkitBuilderObject::BuildWidget() const
{
	if (!BuilderContextWidget.IsValid())
	{
		BuilderContextWidget = SNew(SInventoryToolkitWindow);
	}

	return BuilderContextWidget;
}

TArray<TSharedPtr<FUICommandInfo>> UInventoryToolkitBuilderObject::GetUICommands() const
{
	return
	{
			OpenTool
	};
}

FName UInventoryToolkitBuilderObject::GetMenuName() const
{
	return TEXT("Menu_InventorySample");
}

FName UInventoryToolkitBuilderObject::GetSectionName() const
{
	return TEXT("Section_InventorySample");
}

FName UInventoryToolkitBuilderObject::GetSubSectionName() const
{
	return TEXT("SubSection_InventorySampleEditor");
}

FText UInventoryToolkitBuilderObject::GetSubSection_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UInventoryToolkitBuilderObject::GetSubSection_Label", "InventorySampleEditor");
}

FText UInventoryToolkitBuilderObject::GetSubSection_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UInventoryToolkitBuilderObject::GetSubSection_Tooltips", "Central hub in which designers can create/edit/delete elements from the inventory system.");
}

FName UInventoryToolkitBuilderObject::GetOpenCommandName() const
{
	return TEXT("Open_InventorySample");
}

FText UInventoryToolkitBuilderObject::GetOpenCommand_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UInventoryToolkitBuilderObject::GetOpenCommand_Label", "Open InventorySample");
}

FText UInventoryToolkitBuilderObject::GetOpenCommand_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UInventoryToolkitBuilderObject::GetOpenCommand_Tooltips", "Allow opening the context tooling required for data editing asset specific to the inventory system.");
}
