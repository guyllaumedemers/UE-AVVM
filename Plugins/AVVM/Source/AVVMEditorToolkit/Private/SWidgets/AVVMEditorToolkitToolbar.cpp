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

#include "AVVMEditorBuilderSubsystem.h"
#include "AVVMEditorToolkitModule.h"
#include "AVVMEditorToolkitBuilderObject.h"
#include "LevelEditor.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"

void SAVVMEditorToolkitToolbar::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(MenuBar, SHorizontalBox)
		]
	];

	if (!Handle.IsValid())
	{
		const auto Callback = FOnBuildContextChangedDelegate::FDelegate::CreateRaw(this, &SAVVMEditorToolkitToolbar::OnBuilderObjectsAcquired, InArgs._Core.Get(), InArgs._MenuBarExtenders.Get());
		Handle = UAVVMEditorBuilderSubsystem::Static_CallOrRegisterOnBuildContextChanged(GEditor, Callback);
	}

	OnBuilderObjectsAcquired(nullptr, InArgs._Core.Get(), InArgs._MenuBarExtenders.Get());
}

void SAVVMEditorToolkitToolbar::MakeGenericMenuEntry(const UAVVMEditorToolkitBuilderObject* BuilderObject,
                                                     TSharedPtr<FAVVMEditorToolkit_Core> Core,
                                                     TSharedPtr<FExtender> MenuBarExtenders)
{
	if (!Core.IsValid() || !ensureAlwaysMsgf(IsValid(BuilderObject),
	                                         TEXT("Failed to retrieve valid Builder Object.")))
	{
		return;
	}

	const FName MenuName = BuilderObject->GetMenuName();
	const FName SectionName = BuilderObject->GetSectionName();

	UToolMenus* MenuManager = UToolMenus::Get();
	if (!IsValid(MenuManager))
	{
		return;
	}

	const FToolMenuContext ToolMenuContext
	{
			Core->GetToolkitCommands(),
			MenuBarExtenders,
			const_cast<UAVVMEditorToolkitBuilderObject*>(BuilderObject)
	};

	if (!MenuManager->IsMenuRegistered(MenuName))
	{
		UToolMenu* MenuEntry = MenuManager->RegisterMenu(MenuName);
		BuilderObject->RegisterCommands(Core);

		FToolMenuSection& OptionsSection = MenuEntry->AddSection(SectionName);
		for (const auto& UICommandInfo : BuilderObject->GetUICommands())
		{
			OptionsSection.AddMenuEntryWithCommandList(UICommandInfo, Core->GetToolkitCommands());
		}

		MenuManager->GenerateMenu(MenuName, ToolMenuContext);
	}

	if (!MenuBar.IsValid())
	{
		return;
	}

	MenuBar->AddSlot()
	       .Padding(12.f)
	       .AutoWidth()
	       .AttachWidget(SNew(SComboButton)
	       .HasDownArrow(false)
	       .ContentPadding(FMargin(12.f))
	       .ForegroundColor(FSlateColor::UseForeground())
	       .ButtonStyle(FAppStyle::Get(), "SimpleButton")
	       .ToolTipText(BuilderObject->GetSubSection_Tooltips())
	       .MenuContent()
	       [
		       MenuManager->GenerateWidget(MenuName, ToolMenuContext)
	       ]
	       .ButtonContent()
	       [
		       SNew(STextBlock)
		       .Text(BuilderObject->GetSubSection_Label())
	       ]);
}

void SAVVMEditorToolkitToolbar::OnBuilderObjectsAcquired(const UAVVMEditorToolkitBuilderObject* BuilderContextObject,
                                                         TSharedPtr<FAVVMEditorToolkit_Core> Core,
                                                         TSharedPtr<FExtender> MenuBarExtenders)
{
	if (MenuBar.IsValid())
	{
		MenuBar->ClearChildren();
	}
	
	for (const auto* BuilderObject : UAVVMEditorBuilderSubsystem::Static_GetAllBuilders(GEditor))
	{
		MakeGenericMenuEntry(BuilderObject, Core, MenuBarExtenders);
	}
}
