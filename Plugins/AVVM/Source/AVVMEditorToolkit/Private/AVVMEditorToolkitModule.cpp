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

#include "AVVMEditorToolkitModule.h"

#include "LevelEditor.h"
#include "UMGStyle.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "SWidgets/AVVMEditorToolkitRoot.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "AVVMEditorToolkit"

/**
 *	@gdemers Private namespace for data that may be accessed extern to this source file.
 */
namespace NS_AVVMEditorToolkit
{
	// @gdemers TabId
	const FName MainTab_Name = TEXT("MainTab");
	// @gdemers Section
	const FName MainSection_Name = TEXT("MainSection");
	// @gdemers menu header
	const FText Menu_Label = LOCTEXT("Menu_Label", "AVVMEditorToolkit");
	const FText Menu_Tooltips = LOCTEXT("Menu_Tooltips", "A set of editor tools for helping user setup their project data assets based on AVVM data scheme.");
	const FSlateIcon Menu_TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.TabIcon");
	// @gdemers toolkit
	const FText MenuHub_Label = LOCTEXT("MenuHub_Label", "Hub");
	const FText MenuHub_Tooltips = Menu_Tooltips;
	const FSlateIcon MenuHub_Icon = FSlateIcon(FUMGStyle::GetStyleSetName(), "Designer.TabIcon");
	// @gdemers viewport header
	const FText Viewport_Label = MenuHub_Label;
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
	UI_COMMAND(OpenEditorToolkit_Window, "Hub", "Open AVVMEditorToolkit Hub", EUserInterfaceActionType::Button, FInputChord());
}

void FAVVMEditorToolkit_Commands::BindNewCommand(const FAVVMBindingContext& BindingContext, TSharedPtr<FUICommandInfo>& OutCmd)
{
	FUICommandInfo::MakeCommandInfo(AsShared(),
	                                OutCmd,
	                                BindingContext.CommandName,
	                                BindingContext.CommandLabel,
	                                BindingContext.CommandDesc,
	                                BindingContext.Icon,
	                                BindingContext.UserInterfaceType,
	                                BindingContext.DefaultChord,
	                                BindingContext.AlternateDefaultChord,
	                                BindingContext.Bundle);
}

FAVVMBindingContext FAVVMEditorToolkit_Commands::MakeButtonContext(const FName CmdName, const FText& CmdLabel, const FText& CmdDescription)
{
	return FAVVMBindingContext
	{
			CmdName,
			CmdLabel,
			CmdDescription,
			FSlateIcon(),
			EUserInterfaceActionType::Button,
			FInputChord(),
			FInputChord(),
			NAME_None
	};
}

void FAVVMEditorToolkitModule::StartupModule()
{
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);

	BindCommands();
	RegisterUnderMenuBar();
	RegisterUnderWindowTab();
	RegisterUnderLevelEditorTab();
}

void FAVVMEditorToolkitModule::ShutdownModule()
{
	ToolBarExtensibilityManager.Reset();
	MenuExtensibilityManager.Reset();
	Core.Reset();

	// @gdemers unregister global tab for our main presentation viewport.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(NS_AVVMEditorToolkit::MainTab_Name);
}

TSharedPtr<FExtensibilityManager> FAVVMEditorToolkitModule::GetToolBarExtensibilityManager()
{
	return ToolBarExtensibilityManager;
}

TSharedPtr<FExtensibilityManager> FAVVMEditorToolkitModule::GetMenuExtensibilityManager()
{
	return MenuExtensibilityManager;
}

void FAVVMEditorToolkitModule::BindCommands()
{
	// @gdemers get-set singleton instance for our FUICommandList
	TCommands<FAVVMEditorToolkit_Commands>::Register();

	struct FAVVMEditorActions
	{
		static void OpenEditorToolkitWindow()
		{
			// @gdemers open our tool viewport from the registered tab system so we don't have to handle
			// the viewport lifecycle ourselves.
			FGlobalTabmanager::Get()->TryInvokeTab(NS_AVVMEditorToolkit::MainTab_Name);
		}
	};

	Core = MakeShared<FAVVMEditorToolkit_Core>();
	Core->GetToolkitCommands()->MapAction(FAVVMEditorToolkit_Commands::Get().OpenEditorToolkit_Window,
	                                      FExecuteAction::CreateStatic(&FAVVMEditorActions::OpenEditorToolkitWindow),
	                                      FCanExecuteAction{});
}

void FAVVMEditorToolkitModule::RegisterUnderWindowTab()
{
	const auto MakeWindowTabEntry = [](const class FSpawnTabArgs& SpawnTabArgs,
	                                    int32 TabIndex,
	                                    TSharedPtr<FExtensibilityManager> ExtensibilityManager,
	                                    TSharedPtr<FAVVMEditorToolkit_Core> EditorToolkit)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			.Label(NS_AVVMEditorToolkit::Viewport_Label)
			[
				SNew(SAVVMEditorToolkitRoot)
				.MenuBarExtenders(ExtensibilityManager.IsValid() ? ExtensibilityManager->GetAllExtenders() : nullptr)
				.Core(EditorToolkit)
			];
	};

	TSharedRef<FWorkspaceItem> ToolGroup = WorkspaceMenu::GetMenuStructure()
	                                       .GetStructureRoot()
	                                       ->AddGroup(NS_AVVMEditorToolkit::Menu_Label,
	                                                  NS_AVVMEditorToolkit::Menu_Tooltips,
	                                                  NS_AVVMEditorToolkit::Menu_TabIcon,
	                                                  true);

	FGlobalTabmanager::Get()
			->RegisterNomadTabSpawner(NS_AVVMEditorToolkit::MainTab_Name, FOnSpawnTab::CreateLambda(MakeWindowTabEntry, 0, GetMenuExtensibilityManager(), Core))
			.SetDisplayName(NS_AVVMEditorToolkit::MenuHub_Label)
			.SetTooltipText(NS_AVVMEditorToolkit::MenuHub_Tooltips)
			.SetIcon(NS_AVVMEditorToolkit::MenuHub_Icon)
			.SetGroup(ToolGroup);
}

void FAVVMEditorToolkitModule::RegisterUnderLevelEditorTab() const
{
	auto* GlobalToolMenu = UToolMenus::Get();
	if (!IsValid(GlobalToolMenu) || !Core.IsValid())
	{
		return;
	}

	UToolMenu* ToolkitMenu = GlobalToolMenu->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	if (IsValid(ToolkitMenu))
	{
		const TAttribute<FText>& Label = NS_AVVMEditorToolkit::MenuHub_Label;
		const TAttribute<FText>& Tooltips = NS_AVVMEditorToolkit::MenuHub_Tooltips;
		const FSlateIcon& Icon = NS_AVVMEditorToolkit::MenuHub_Icon;

		FToolMenuSection& Section = ToolkitMenu->AddSection(NS_AVVMEditorToolkit::MainSection_Name);
		Section.AddMenuEntryWithCommandList(FAVVMEditorToolkit_Commands::Get().OpenEditorToolkit_Window, Core->GetToolkitCommands(), Label, Tooltips, Icon);
	}
}

void FAVVMEditorToolkitModule::RegisterUnderMenuBar() const
{
	if (Core.IsValid())
	{
		auto& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		FAVVMExtensibilityContext Ctx;
		Ctx.ExtensionHook = TEXT("Help");
		Ctx.Position = EExtensionHook::Position::After;
		Ctx.Label = NS_AVVMEditorToolkit::Menu_Label;
		Ctx.Tooltips = NS_AVVMEditorToolkit::Menu_Tooltips;
		Ctx.ExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
		Ctx.Commands = {FAVVMEditorToolkit_Commands::Get().OpenEditorToolkit_Window};
		Ctx.CommandList = Core->GetToolkitCommands();
		FAVVMEditorToolkitModule::MakeNewPullDownMenuEntries(Ctx);
	}
}

void FAVVMEditorToolkitModule::MakeNewPullDownMenuEntries(const FAVVMExtensibilityContext& Ctx)
{
	if (!Ctx.ExtensibilityManager.IsValid() || Ctx.Commands.IsEmpty())
	{
		return;
	}

	TSharedRef<FExtender> MenuBarExtender(new FExtender());
	Ctx.ExtensibilityManager->AddExtender(MenuBarExtender);

	const auto Callback = FMenuBarExtensionDelegate::CreateStatic(&FAVVMEditorToolkitModule::MakeNewPullDownMenu, Ctx);
	MenuBarExtender->AddMenuBarExtension(Ctx.ExtensionHook, Ctx.Position, Ctx.CommandList, Callback);
}

void FAVVMEditorToolkitModule::MakeNewPullDownMenu(FMenuBarBuilder& Builder, FAVVMExtensibilityContext Ctx)
{
	const auto Callback = FNewMenuDelegate::CreateStatic(&FAVVMEditorToolkitModule::MakeNewMenuEntry, Ctx.Commands);
	Builder.AddPullDownMenu(Ctx.Label, Ctx.Tooltips, Callback);
}

void FAVVMEditorToolkitModule::MakeNewMenuEntry(FMenuBuilder& Builder, TArray<TSharedPtr<FUICommandInfo>> Commands)
{
	for (const auto& Command : Commands)
	{
		Builder.AddMenuEntry(Command);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAVVMEditorToolkitModule, AVVMEditorToolkit)
