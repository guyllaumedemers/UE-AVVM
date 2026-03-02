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
		void OpenRecentFiles()
		{
			UE_LOG(LogTemp, Log, TEXT("NS_AVVMDataTableEditor::OpenRecentFiles"));
		}

		void OpenFile()
		{
			UE_LOG(LogTemp, Log, TEXT("NS_AVVMDataTableEditor::OpenFile"));
		}

		void SaveAll()
		{
			UE_LOG(LogTemp, Log, TEXT("NS_AVVMDataTableEditor::SaveAll"));
		}
	}
}

TSharedPtr<SWidget> UInventoryToolkitBuilderObject::BuildWidget()
{
	// TODO @gdemers create a viewport slate widget thats loads the required Data Table for
	// populating our inventory system.
	return nullptr;
}

void UInventoryToolkitBuilderObject::RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core)
{
	if (OpenDataTableEditor_RecentFiles.IsValid() || OpenDataTableEditor_OpenFile.IsValid() || OpenDataTableEditor_SaveAll.IsValid())
	{
		return;
	}

	BIND_NEW_BUTTON_COMMAND(OpenDataTableEditor_RecentFiles,
	                        FName("InventorySample_OpenRecentFiles"),
	                        LOCTEXT("Open Recent Files", "Open Recent Files"),
	                        LOCTEXT("Lorem Ipsum", "Lorem Ipsum"));

	BIND_NEW_BUTTON_COMMAND(OpenDataTableEditor_OpenFile,
	                        FName("InventorySample_OpenFile"),
	                        LOCTEXT("Open File", "Open Recent Files"),
	                        LOCTEXT("Lorem Ipsum", "Lorem Ipsum"));

	BIND_NEW_BUTTON_COMMAND(OpenDataTableEditor_SaveAll,
	                        FName("InventorySample_SaveAll"),
	                        LOCTEXT("Save All", "Save All"),
	                        LOCTEXT("Lorem Ipsum", "Lorem Ipsum"));

	if (Core.IsValid())
	{
		Core->GetToolkitCommands()->MapAction(OpenDataTableEditor_RecentFiles, FExecuteAction::CreateStatic(&NS_InventoryDataTableEditor::OpenRecentFiles), FCanExecuteAction());
		Core->GetToolkitCommands()->MapAction(OpenDataTableEditor_OpenFile, FExecuteAction::CreateStatic(&NS_InventoryDataTableEditor::OpenFile), FCanExecuteAction());
		Core->GetToolkitCommands()->MapAction(OpenDataTableEditor_SaveAll, FExecuteAction::CreateStatic(&NS_InventoryDataTableEditor::SaveAll), FCanExecuteAction());
	}
}

TArray<TSharedPtr<FUICommandInfo>> UInventoryToolkitBuilderObject::GetUICommands() const
{
	return
	{
			OpenDataTableEditor_OpenFile,
			OpenDataTableEditor_RecentFiles,
			OpenDataTableEditor_SaveAll,
	};
}
#endif

#undef LOCTEXT_NAMESPACE
