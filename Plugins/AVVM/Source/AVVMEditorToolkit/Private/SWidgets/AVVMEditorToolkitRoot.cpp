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
#include "Components/VerticalBox.h"
#include "Widgets/SOverlay.h"

void SAVVMEditorToolkitRoot::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SAssignNew(Toolbar, SAVVMEditorToolkitToolbar)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.FillContentHeight(1.f)
			[
				SAssignNew(WindowContext, SAVVMEditorToolkitContextWindow)
			]
		]
	];

	if (ensureAlwaysMsgf(Toolbar.IsValid(), TEXT("Invalid Toolbar.")))
	{
		Toolbar->Setup(InArgs._Core, InArgs._MenuBarExtenders);
	}

	if (ensureAlwaysMsgf(WindowContext.IsValid(), TEXT("Invalid Window Context.")))
	{
		WindowContext->Setup(InArgs._Core);
	}
}
