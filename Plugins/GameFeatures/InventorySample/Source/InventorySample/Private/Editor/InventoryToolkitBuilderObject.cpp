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
#include "Editor/InventoryToolkitBuilderObject.h"

#define LOCTEXT_NAMESPACE "AVVMEditorToolkit"

#if WITH_EDITORONLY_DATA
#include "AVVMEditorToolkit.h"
#include "InventoryToolkitWindow.h"
#include "Framework/Commands/UICommandInfo.h"

/**
 *	@gdemers Private namespace for reusable api specific to the AVVM DataTableEditor toolkit.
 *	
 *	Note : Inline namespace expose the internal content to the parent scope, allowing me
 *	to declare a name for accessing the nested content without affecting linkage properties.
 */
inline namespace NS_InventoryDataTableEditor
{
	// @gdemers anonymous namespace provide support for implicit internal linkage (no static required),
	// and expose to the parent scope.
	namespace
	{
		void OpenTool(UInventoryToolkitBuilderObject* BuilderContextObject)
		{
			if (IsValid(BuilderContextObject))
			{
				UE_LOG(LogTemp, Log, TEXT("NS_AVVMDataTableEditor::OpenTool"));
				BuilderContextObject->SetActiveSelf();
			}
		}
	}
}

TSharedPtr<SWidget> UInventoryToolkitBuilderObject::BuildWidget()
{
	BuilderContextWidget = SNew(SInventoryToolkitWindow);
	return BuilderContextWidget;
}

void UInventoryToolkitBuilderObject::RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core)
{
	if (OpenTool.IsValid())
	{
		return;
	}

	BIND_NEW_BUTTON_COMMAND(OpenTool,
	                        FName("InventorySample_OpenTool"),
	                        NSLOCTEXT("AVVMEditorToolkit", "Open InventorySample", "Open InventorySample"),
	                        NSLOCTEXT("AVVMEditorToolkit", "Lorem Ipsum", "Lorem Ipsum"));

	if (Core.IsValid())
	{
		Core->GetToolkitCommands()->MapAction(OpenTool, FExecuteAction::CreateStatic(&NS_InventoryDataTableEditor::OpenTool, this), FCanExecuteAction());
	}
}

TArray<TSharedPtr<FUICommandInfo>> UInventoryToolkitBuilderObject::GetUICommands() const
{
	return
	{
			OpenTool
	};
}
#endif

#undef LOCTEXT_NAMESPACE
