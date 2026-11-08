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
#include "WeaponToolkitBuilderObject.h"

#include "WeaponToolkitWindow.h"

TSharedPtr<SWidget> UWeaponToolkitBuilderObject::BuildWidget() const
{
	if (!BuilderContextWidget.IsValid())
	{
		BuilderContextWidget = SNew(SWeaponToolkitWindow);
	}

	return BuilderContextWidget;
}

TArray<TSharedPtr<FUICommandInfo>> UWeaponToolkitBuilderObject::GetUICommands() const
{
	return
	{
			OpenTool
	};
}

FName UWeaponToolkitBuilderObject::GetMenuName() const
{
	return TEXT("Menu_WeaponSample");
}

FName UWeaponToolkitBuilderObject::GetSectionName() const
{
	return TEXT("Section_WeaponSample");
}

FName UWeaponToolkitBuilderObject::GetSubSectionName() const
{
	return TEXT("SubSection_WeaponSample");
}

FText UWeaponToolkitBuilderObject::GetSubSection_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UWeaponToolkitBuilderObject::GetSubSection_Label", "WeaponSampleEditor");
}

FText UWeaponToolkitBuilderObject::GetSubSection_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UWeaponToolkitBuilderObject::GetSubSection_Tooltips", "Central hub in which designers can create/edit/delete elements from the weapon system.");
}

FName UWeaponToolkitBuilderObject::GetOpenCommandName() const
{
	return TEXT("Open_WeaponSample");
}

FText UWeaponToolkitBuilderObject::GetOpenCommand_Label() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UWeaponToolkitBuilderObject::GetOpenCommand_Label", "Open WeaponSample");
}

FText UWeaponToolkitBuilderObject::GetOpenCommand_Tooltips() const
{
	return NSLOCTEXT("AVVMEditorToolkit", "UWeaponToolkitBuilderObject::GetOpenCommand_Tooltips", "Allow opening the context tooling required for data editing asset specific to the weapon system.");
}
