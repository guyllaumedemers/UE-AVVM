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

#include "AVVMEditorToolkit.h"

#include "UMGStyle.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "SWidgets/AVVMEditorToolkitRoot.h"

#define LOCTEXT_NAMESPACE "FAVVMEditorToolkit"

void FAVVMEditorToolkit::StartupModule()
{
	const FText Header = LOCTEXT("ToolsCategory", "AVVMEditorToolkit");
	const FText Tooltips = LOCTEXT("ToolsCategory_Tooltip", "A set of editor tools for helping user setup their project data assets based on AVVM data scheme.");
	const FSlateIcon TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.TabIcon");
	const FSlateIcon Toolkit1_Icon = FSlateIcon(FUMGStyle::GetStyleSetName(), "Designer.TabIcon");

	TSharedRef<FWorkspaceItem> ToolCategory = WorkspaceMenu::GetMenuStructure().GetToolsCategory();
	TSharedRef<FWorkspaceItem> ToolGroup = ToolCategory->AddGroup(Header, Tooltips, TabIcon, true);

	const auto Callback = FOnSpawnTab::CreateRaw(this, &FAVVMEditorToolkit::OnSpawnPluginTab, 0);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TEXT("Toolkit1"), Callback)
	                        .SetDisplayName(LOCTEXT("Toolkit1", "DataTableEditor"))
	                        .SetTooltipText(LOCTEXT("Toolkit1_Tooltips", "Open a Data Table Editor to generate project data following AVVM plugin requirements."))
	                        .SetGroup(ToolGroup)
	                        .SetIcon(Toolkit1_Icon);
}

void FAVVMEditorToolkit::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TEXT("Toolkit1"));
}

FText FAVVMEditorToolkit::GetTabLabel(int32 TabIndex)
{
	return FText::Format(LOCTEXT("ToolkitName", "Toolkit{0}"), FText::AsNumber(TabIndex + 1));
}

TSharedRef<class SDockTab> FAVVMEditorToolkit::OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs,
                                                                int32 TabIndex)
{
	TSharedRef<SAVVMEditorToolkitRoot> NewPalette = SNew(SAVVMEditorToolkitRoot, TabIndex);

	const auto Callback = TAttribute<FText>::FGetter::CreateRaw(this, &FAVVMEditorToolkit::GetTabLabel, TabIndex);
	TAttribute<FText> TabLabel = TAttribute<FText>::Create(Callback);

	TSharedRef<SDockTab> ResultTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(TabLabel)
		[
			NewPalette
		];

	return ResultTab;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAVVMEditorToolkit, AVVMEditorToolkit)
