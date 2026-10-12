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
#include "SWidgets/AVVMEditorToolkitOutputPathPicker.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Components/VerticalBox.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Text/STextBlock.h"

void SAVVMEditorToolkitOutputPathPicker::Construct(const FArguments& InArgs)
{
	TSharedPtr<SWidget> PathPicker{nullptr};
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		// Configure the path picker
		FPathPickerConfig PathPickerConfig;
		PathPickerConfig.DefaultPath = TEXT("/Game");
		PathPickerConfig.OnPathSelected = InArgs._OnPathSelected;

		// Load the Content Browser module
		auto& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		PathPicker = ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig);
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 1.0f, 20.0f, 1.0f)
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
			.Text(InArgs._OutputHeader)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 1.0f, 20.0f, 1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			[
				PathPicker.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SEditableTextBox)
					.Justification(ETextJustify::Center)
					.Text(InArgs._EditableItemText)
					.OnTextChanged(InArgs._OnEditableTextChanged)
					.IsEnabled(InArgs._IsEditableTextEnabled)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Left)
					.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
					.Text(InArgs._OutputAssetPathText)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Left)
					.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
					.Text(InArgs._OutputItemText)
				]
			]
		]
	];
}
