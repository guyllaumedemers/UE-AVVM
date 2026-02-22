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
#include "AVVMEditorToolbar.h"

#include "AVVMEditorToolkit.h"
#include "UMGStyle.h"
#include "Styling/ToolBarStyle.h"

#define LOCTEXT_NAMESPACE "AVVMEditorToolkit_ToolBar"

/**
 *	@gdemers Private namespace for data that may be accessed extern to this source file.
 */
namespace NS_AVVMEditorToolkit_ToolBar
{
	// @gdemers menu
	const FName ToolkitMenu_Name = TEXT("Toolkit_Menu");
	// @gdemers sections
	const FName DataTableEditorSection_Name = TEXT("DataTableEditor_Section");
	const FName DataTableEditorSubSection_Name = TEXT("DataTableEditor_SubSection");
	// @gdemers loc text
	const FText DataTableEditor_Label = LOCTEXT("AVVMEditorToolkit_ToolBar", "DataTable Editor");
	const FText DataTableEditor_Tooltips = LOCTEXT("AVVMEditorToolkit_ToolBar", "Lorem ipsum");
	const FSlateIcon DataTableEditor_Icon = FSlateIcon(FUMGStyle::GetStyleSetName(), "Designer.TabIcon");
}

TSharedRef<SWidget> FAVVMEditorToolbar::MakeSecondaryToolbar(TSharedRef<FUICommandList> InCommandList,
                                                             TSharedPtr<FExtender> MenuBarExtenders)
{
	FToolMenuContext MenuContext(InCommandList, MenuBarExtenders);

	UToolMenu* ToolkitMenuBar = UToolMenus::Get()->RegisterMenu(NS_AVVMEditorToolkit_ToolBar::ToolkitMenu_Name, NAME_None, EMultiBoxType::MenuBar);
	if (IsValid(ToolkitMenuBar))
	{
		static const FName MainMenuStyleName("WindowMenuBar");
		ToolkitMenuBar->SetStyleSet(&FAppStyle::Get());
		ToolkitMenuBar->StyleName = MainMenuStyleName;
		
		{
			// @gdemers build all the relevant tools we expect. Note : general purpose tools only. the context in which
			// they are used may differ, but the asset target should remain the same.
			MakeDataTableEditor(*ToolkitMenuBar);
		}
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window").BackgroundBrush)
		]
		+ SOverlay::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBorder)
				.Padding(0.f)
				.BorderImage(FAppStyle::Get().GetBrush("NoBorder"))
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
				[
					UToolMenus::Get()->GenerateWidget(NS_AVVMEditorToolkit_ToolBar::ToolkitMenu_Name, MenuContext)
				]
			]
		];
}

void FAVVMEditorToolbar::MakeNewSubMenu(const FAVVMSubMenuContext& Ctx, UToolMenu& Menu)
{
	FToolMenuSection& NewSection = Menu.AddSection(Ctx.SectionName);
	NewSection.AddSubMenu(Ctx.SubSectionName,
	                      Ctx.SubSection_Label,
	                      Ctx.SubSection_Tooltips,
	                      FNewToolMenuDelegate::CreateStatic(&FAVVMEditorToolbar::FillNewSubMenu, Ctx.SubSectionName, Ctx.Commands),
	                      true,
	                      Ctx.SubSection_Icon);
}

void FAVVMEditorToolbar::FillNewSubMenu(UToolMenu* InMenu, FName SectionName, TArray<TSharedPtr<FUICommandInfo>> Commands)
{
	if (!IsValid(InMenu))
	{
		return;
	}

	for (const auto& Command : Commands)
	{
		FToolMenuSection& MenuBarSection = InMenu->FindOrAddSection(SectionName);
		MenuBarSection.AddMenuEntry(Command);
	}
}

void FAVVMEditorToolbar::MakeDataTableEditor(UToolMenu& InMenu)
{
	FAVVMSubMenuContext Ctx;
	Ctx.SectionName = NS_AVVMEditorToolkit_ToolBar::DataTableEditorSection_Name;
	Ctx.SubSectionName = NS_AVVMEditorToolkit_ToolBar::DataTableEditorSection_Name;
	Ctx.SubSection_Label = NS_AVVMEditorToolkit_ToolBar::DataTableEditor_Label;
	Ctx.SubSection_Tooltips = NS_AVVMEditorToolkit_ToolBar::DataTableEditor_Tooltips;
	Ctx.SubSection_Icon = NS_AVVMEditorToolkit_ToolBar::DataTableEditor_Icon;

	Ctx.Commands =
	{
			FAVVMEditorToolkit_Commands::Get().OpenDataTableEditor_OpenFile,
			FAVVMEditorToolkit_Commands::Get().OpenDataTableEditor_RecentFiles,
			FAVVMEditorToolkit_Commands::Get().OpenDataTableEditor_SaveAll,
	};

	MakeNewSubMenu(Ctx, InMenu);
}

#undef LOCTEXT_NAMESPACE
