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
#include "AVVMEditorToolkitToolbar.h"

#include "AVVMEditorToolkit.h"
#include "ToolMenu.h"
#include "Styling/ToolBarStyle.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

#define LOCTEXT_NAMESPACE "AVVMEditorToolkit_ToolBar"

/**
 *	@gdemers Private namespace for data that may be accessed extern to this source file.
 */
namespace NS_AVVMEditorToolkit_ToolBar
{
	// @gdemers menu
	const FName ToolkitMenu_Name = TEXT("Toolkit_Menu");
}

void SAVVMEditorToolkitToolbar::Construct(const FArguments& InArgs)
{
}

void SAVVMEditorToolkitToolbar::Setup(TSharedPtr<FAVVMEditorToolkit_Core> Core,
                                      TSharedPtr<FExtender> MenuBarExtenders)
{
	if (!Core.IsValid() || !MenuBarExtenders.IsValid())
	{
		return;
	}

	UToolMenu* ToolkitMenuBar = UToolMenus::Get()->RegisterMenu(NS_AVVMEditorToolkit_ToolBar::ToolkitMenu_Name, NAME_None, EMultiBoxType::MenuBar);
	if (IsValid(ToolkitMenuBar))
	{
		static const FName MainMenuStyleName("WindowMenuBar");
		ToolkitMenuBar->SetStyleSet(&FAppStyle::Get());
		ToolkitMenuBar->StyleName = MainMenuStyleName;

		for (const auto& BuilderObject : Core->GetBuilderContexts())
		{
			// @gdemers build module extensions using the provided builder object.
			MakeGenericMenuEntry(BuilderObject.Get(), Core, *ToolkitMenuBar);
		}
	}

	const auto Toolbar = UToolMenus::Get()->GenerateWidget(
	                                                       NS_AVVMEditorToolkit_ToolBar::ToolkitMenu_Name,
	                                                       FToolMenuContext{Core->GetToolkitCommands(), MenuBarExtenders});

	ChildSlot
	[
		SNew(SOverlay)
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
					Toolbar
				]
			]
		]
	];
}

void SAVVMEditorToolkitToolbar::MakeGenericMenuEntry(UAVVMEditorToolkitBuilderObject* BuilderObject,
                                                     TSharedPtr<FAVVMEditorToolkit_Core> Core,
                                                     UToolMenu& OutMenu)
{
	if (!IsValid(BuilderObject))
	{
		return;
	}

	FAVVMBuilderInfo OutBuilderInfo;
	BuilderObject->GetBuilderInfo(OutBuilderInfo);
	BuilderObject->RegisterCommands(Core);

	FToolMenuSection& NewSection = OutMenu.AddSection(OutBuilderInfo.SectionName);
	NewSection.AddSubMenu(OutBuilderInfo.SubSectionName,
	                      OutBuilderInfo.SubSection_Label,
	                      OutBuilderInfo.SubSection_Tooltips,
	                      FNewToolMenuDelegate::CreateStatic(&SAVVMEditorToolkitToolbar::FillNewSubMenu, OutBuilderInfo.SubSectionName, OutBuilderInfo.Commands),
	                      true,
	                      OutBuilderInfo.SubSection_Icon);
}

void SAVVMEditorToolkitToolbar::FillNewSubMenu(UToolMenu* InMenu, FName SectionName, TArray<TSharedPtr<FUICommandInfo>> Commands)
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

#undef LOCTEXT_NAMESPACE
