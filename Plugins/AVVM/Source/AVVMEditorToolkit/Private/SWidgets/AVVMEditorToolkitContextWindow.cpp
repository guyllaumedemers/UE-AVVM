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
#include "AVVMEditorToolkitContextWindow.h"

#include "AVVMEditorBuilderSubsystem.h"
#include "AVVMEditorToolkitBuilderObject.h"
#include "Editor.h"

void SAVVMEditorToolkitContextWindow::Construct(const FArguments& InArgs)
{
}

void SAVVMEditorToolkitContextWindow::Setup(TSharedPtr<FAVVMEditorToolkit_Core> Core)
{
	if (!Handle.IsValid())
	{
		const auto Callback = FOnBuildContextChangedDelegate::FDelegate::CreateRaw(this, &SAVVMEditorToolkitContextWindow::OnBuildContextChanged);
		Handle = UAVVMEditorBuilderSubsystem::Static_CallOrRegisterOnBuildContextChanged(GEditor, Callback);
	}

	UAVVMEditorToolkitBuilderObject* BuilderObject = UAVVMEditorBuilderSubsystem::Static_GetActiveBuilder(GEditor);
	OnBuildContextChanged(BuilderObject);
}

void SAVVMEditorToolkitContextWindow::OnBuildContextChanged(UAVVMEditorToolkitBuilderObject* BuilderContextObject)
{
	if (!IsValid(BuilderContextObject))
	{
		return;
	}

	TSharedPtr<SWidget> Widget = BuilderContextObject->BuildWidget();
	if (!Widget.IsValid())
	{
		return;
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			Widget->AsShared()
		]
	];
}
