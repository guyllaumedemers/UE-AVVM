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
#include "SWidgets/AVVMEditorToolkitRequiredUserInput.h"

#include "Components/VerticalBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

void SAVVMEditorToolkitRequiredUserInput::Construct(const FArguments& InArgs)
{
	OnTextCommitted = InArgs._OnTextCommited;
	bShouldCloseWindowOnCommit = InArgs._bShouldCloseWindowOnCommit.Get();
	
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
				SNew(SEditableTextBox)
				.Justification(ETextJustify::Left)
				.OnKeyDownHandler(this, &SAVVMEditorToolkitRequiredUserInput::OnKeyDownHandler)
				.OnTextChanged(this, &SAVVMEditorToolkitRequiredUserInput::OnTextChanged)
				.HintText(NSLOCTEXT("AVVMEditorToolkit", "RequiredInput_HintText", "Press Enter to confirm name selection."))
				.Text(NSLOCTEXT("AVVMEditorToolkit", "RequiredInput_DefaultText", "..."))
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

FReply SAVVMEditorToolkitRequiredUserInput::OnKeyDownHandler(const FGeometry& Geometry,
                                                             const FKeyEvent& Event)
{
	if (Event.GetKey() != EKeys::Enter)
	{
		return FReply::Unhandled();
	}

	OnTextCommitted.ExecuteIfBound(InputText, ETextCommit::OnEnter);
	if (!bShouldCloseWindowOnCommit)
	{
		return FReply::Handled();
	}

	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ensureAlwaysMsgf(ParentWindow.IsValid(), TEXT("Failed to find bound SWindow.")))
	{
		ParentWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride());
		ParentWindow->RequestDestroyWindow();
	}

	return FReply::Handled();
}

void SAVVMEditorToolkitRequiredUserInput::OnTextChanged(const FText& NewText)
{
	InputText = NewText;
}
