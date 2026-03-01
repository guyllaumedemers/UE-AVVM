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
#include "AVVMEditorToolkitRoot.h"

#include "AVVMEditorToolkitContextWindow.h"
#include "AVVMEditorToolkitToolbar.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

void SAVVMEditorToolkitRoot::Construct(const FArguments& InArgs)
{
}

void SAVVMEditorToolkitRoot::Setup(TSharedPtr<FAVVMEditorToolkit_Core> Core,
                                   TSharedPtr<FExtender> MenuBarExtenders)
{
	SecondaryMenuToolbarWidget = SNew(SBorder)
		.Padding(0.f)
		.BorderImage(FAppStyle::Get().GetBrush("NoBorder"));

	{
		const auto NewToolbar = SNew(SAVVMEditorToolkitToolbar);
		NewToolbar->Setup(Core, MenuBarExtenders);
		SecondaryMenuToolbarWidget->SetContent(NewToolbar);
	}

	SecondaryContextWindowWidget = SNew(SBorder)
		.Padding(0.f)
		.BorderImage(FAppStyle::Get().GetBrush("NoBorder"));

	{
		const auto NewWindowContext = SNew(SAVVMEditorToolkitContextWindow);
		NewWindowContext->Setup(Core);
		SecondaryContextWindowWidget->SetContent(NewWindowContext);
	}
	
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
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SecondaryMenuToolbarWidget.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 2.0f))
			.FillContentHeight(1.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::Get().GetBrush("NoBorder"))
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
				[
					SecondaryContextWindowWidget.ToSharedRef()
				]
			]
		]
	];
}
