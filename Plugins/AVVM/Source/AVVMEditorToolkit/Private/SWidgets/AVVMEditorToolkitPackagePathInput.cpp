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
#include "SWidgets/AVVMEditorToolkitPackagePathInput.h"

#include "Components/VerticalBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

void SAVVMEditorToolkitRequiredUserInput::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), "LargeText")
				.TransformPolicy(ETextTransformPolicy::ToUpper)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "RequiredUserInput", "Step1: Choose an Asset name:"))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), "NormalText")
				.TransformPolicy(ETextTransformPolicy::ToUpper)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "Important", "Press Enter to confirm name selection."))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), "NormalText")
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Text(NSLOCTEXT("AVVMEditorToolkit", "RequiredInput", "Note : Asset names are standardized under the hood."))
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SEditableTextBox)
				.Justification(ETextJustify::Left)
				.OnTextCommitted(InArgs._OnTextCommited)
				.Text(NSLOCTEXT("AVVMEditorToolkit", "DefaultText", "..."))
			]
		]
	];
}

FText SAVVMEditorToolkitRequiredUserInput::GetModalMessage_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "DataEditor_ClosureMessage", "Are you sure you want to close this window?");
}

FText SAVVMEditorToolkitRequiredUserInput::GetModalTitle_OnClosure() const
{
	return NSLOCTEXT("AVVMEditorToolkitDataEditor", "DataEditor_ClosureTitle", "Missing Source!");
}
