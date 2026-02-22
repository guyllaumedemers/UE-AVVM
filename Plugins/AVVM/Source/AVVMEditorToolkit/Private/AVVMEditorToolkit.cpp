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

#include "LevelEditor.h"
#include "UMGStyle.h"
#include "ToolMenu.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "SWidgets/AVVMEditorToolkitRoot.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AVVMEditorToolkit"

/**
 *	@gdemers Private namespace for data that may be accessed extern to this source file.
 */
namespace NS_AVVMEditorToolkit
{
	// @gdemers menu header
	const FText Menu_Label = LOCTEXT("Menu_Label", "AVVMEditorToolkit");
	const FText Menu_Tooltips = LOCTEXT("Menu_Tooltips", "A set of editor tools for helping user setup their project data assets based on AVVM data scheme.");
	const FSlateIcon Menu_TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.TabIcon");
	// @gdemers viewport header
	const FText Viewport_Label = LOCTEXT("Viewport_Label", "Core");
	// @gdemers TabId
	const FName MainTab = TEXT("MainTab");
	// @gdemers toolkit
	const FText DataTableEditor_Label = LOCTEXT("DataTableEditor_Label", "DataTableEditor");
	const FText DataTableEditor_Tooltips = LOCTEXT("DataTableEditor_Tooltips", "Open a Data Table Editor to generate project data following AVVM plugin requirements.");
	const FSlateIcon DataTableEditor_DataTableIcon = FSlateIcon(FUMGStyle::GetStyleSetName(), "Designer.TabIcon");
}

FLinearColor FAVVMEditorToolkit_Core::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FAVVMEditorToolkit_Core::GetToolkitFName() const
{
	return TEXT("FAVVMEditorToolkit_Core");
}

FText FAVVMEditorToolkit_Core::GetBaseToolkitName() const
{
	return FText::GetEmpty();
}

FString FAVVMEditorToolkit_Core::GetWorldCentricTabPrefix() const
{
	return FString("FAVVMEditorToolkit_Core");
}

FAVVMEditorToolkit_Commands::FAVVMEditorToolkit_Commands()
	: TCommands("FAVVMEditorToolkit_Commands", FText(), "", FAppStyle::GetAppStyleSetName())
{
}

void FAVVMEditorToolkit_Commands::RegisterCommands()
{
	UI_COMMAND(OpenEditorToolkit, "Open AVVMEditorToolkit", "Open AVVMEditorToolkit", EUserInterfaceActionType::Button, FInputChord());
}

void FAVVMEditorToolkitModule::StartupModule()
{
	{
		// TODO @gdemers expend Managers populating the main viewport that will be created for the tool main presentation.
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	}
	
	{
		BindCommands();
		RegisterUnderWindowTab();
		RegisterUnderLevelEditorTab();
		AppendMenuBar();
	}
}

void FAVVMEditorToolkitModule::ShutdownModule()
{
	{
		ToolBarExtensibilityManager.Reset();
		MenuExtensibilityManager.Reset();
	}
	
	{
		// @gdemers unregister global tab for our main presentation viewport.
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(NS_AVVMEditorToolkit::MainTab);
	}
}

TSharedPtr<FExtensibilityManager> FAVVMEditorToolkitModule::GetToolBarExtensibilityManager()
{
	return ToolBarExtensibilityManager;
}

TSharedPtr<FExtensibilityManager> FAVVMEditorToolkitModule::GetMenuExtensibilityManager()
{
	return MenuExtensibilityManager;
}

void FAVVMEditorToolkitModule::RegisterUnderWindowTab()
{
	struct FAVVMWindowTabBuilder
	{
		static TSharedRef<class SDockTab> MakeWindowTabEntry(const class FSpawnTabArgs& SpawnTabArgs, int32 TabIndex)
		{
			const auto NewPalette = SNew(SAVVMEditorToolkitRoot, TabIndex);
			TSharedRef<SDockTab> ResultTab = SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				.Label(NS_AVVMEditorToolkit::Viewport_Label)
				[
					NewPalette
				];

			return ResultTab;
		}
	};

	TSharedRef<FWorkspaceItem> ToolGroup = WorkspaceMenu::GetMenuStructure()
	                                       .GetStructureRoot()
	                                       ->AddGroup(NS_AVVMEditorToolkit::Menu_Label,
	                                                  NS_AVVMEditorToolkit::Menu_Tooltips,
	                                                  NS_AVVMEditorToolkit::Menu_TabIcon,
	                                                  true);

	FGlobalTabmanager::Get()
			->RegisterNomadTabSpawner(NS_AVVMEditorToolkit::MainTab, FOnSpawnTab::CreateStatic(&FAVVMWindowTabBuilder::MakeWindowTabEntry, 0))
			.SetDisplayName(NS_AVVMEditorToolkit::DataTableEditor_Label)
			.SetTooltipText(NS_AVVMEditorToolkit::DataTableEditor_Tooltips)
			.SetGroup(ToolGroup)
			.SetIcon(NS_AVVMEditorToolkit::DataTableEditor_DataTableIcon);
}

void FAVVMEditorToolkitModule::RegisterUnderLevelEditorTab()
{
	auto* GlobalToolMenu = UToolMenus::Get();
	if (!IsValid(GlobalToolMenu) || !Core.IsValid())
	{
		return;
	}

	UToolMenu* ToolkitMenu = GlobalToolMenu->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	if (IsValid(ToolkitMenu))
	{
		const TAttribute<FText>& Label = NS_AVVMEditorToolkit::DataTableEditor_Label;
		const TAttribute<FText>& Tooltips = NS_AVVMEditorToolkit::DataTableEditor_Tooltips;
		const FSlateIcon& Icon = NS_AVVMEditorToolkit::DataTableEditor_DataTableIcon;

		const auto& Commands = FAVVMEditorToolkit_Commands::Get();
		FToolMenuSection& Section = ToolkitMenu->AddSection("Dummy_Section");
		const auto ToolEntry = FToolMenuEntry::InitMenuEntryWithCommandList(Commands.OpenEditorToolkit, Core->GetToolkitCommands(), Label, Tooltips, Icon);
		Section.AddEntry(ToolEntry);
	}
}

void FAVVMEditorToolkitModule::AppendMenuBar()
{
	struct FAVVMMenuBarEntryBuilder
	{
		static void MakeMenuBarEntry(FMenuBuilder& Builder)
		{
			const FAVVMEditorToolkit_Commands& Commands = FAVVMEditorToolkit_Commands::Get();
			Builder.AddMenuEntry(Commands.OpenEditorToolkit);
		}
	};

	struct FAVVMMenuBarBuilder
	{
		static void MakeMenuBar(FMenuBarBuilder& Builder)
		{
			const TAttribute<FText>& Label = NS_AVVMEditorToolkit::Menu_Label;
			const TAttribute<FText>& Tooltips = NS_AVVMEditorToolkit::Menu_Tooltips;
			Builder.AddPullDownMenu(Label, Tooltips, FNewMenuDelegate::CreateStatic(&FAVVMMenuBarEntryBuilder::MakeMenuBarEntry));
		}
	};

	if (!Core.IsValid())
	{
		return;
	}

	auto& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedRef<FExtender> MenuBarExtender(new FExtender());
	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, Core->GetToolkitCommands(), FMenuBarExtensionDelegate::CreateStatic(&FAVVMMenuBarBuilder::MakeMenuBar));

	auto NewMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
	if (NewMenuExtensibilityManager.IsValid())
	{
		NewMenuExtensibilityManager->AddExtender(MenuBarExtender);
	}
}

void FAVVMEditorToolkitModule::BindCommands()
{
	{
		// @gdemers get-set singleton instance for our FUICommandList
		TCommands<FAVVMEditorToolkit_Commands>::Register();
	}
	
	struct FAVVMTabSpawner
	{
		static void OpenEditorToolkitWindow()
		{
			// @gdemers open our tool viewport from the registered tab system so we don't have to handle
			// the viewport lifecycle ourselves.
			FGlobalTabmanager::Get()->TryInvokeTab(NS_AVVMEditorToolkit::MainTab);
		}
	};
	
	{
		Core = MakeShared<FAVVMEditorToolkit_Core>();
		FAVVMEditorToolkit_Commands& Commands = FAVVMEditorToolkit_Commands::Get();
		Core->GetToolkitCommands()->MapAction(Commands.OpenEditorToolkit, FExecuteAction::CreateStatic(&FAVVMTabSpawner::OpenEditorToolkitWindow), FCanExecuteAction());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAVVMEditorToolkitModule, AVVMEditorToolkit)
