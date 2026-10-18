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
	OnPathSelected = InArgs._OnPathSelected;

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
				.Justification(ETextJustify::Left)
				.Font(FAppStyle::GetFontStyle("NormalFontBold"))
				.Text(InArgs._AssetPathHeaderText)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				[
					SNew(SBorder)
					.Padding(6.0f)
					.OnMouseDoubleClick(this, &SAVVMEditorToolkitOutputPathPicker::OnMouseDoubleClicked)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(8.0f, 0.0f, 8.0f, 0.0f)
						.AutoWidth()
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush("Icons.Folder"))
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(8.0f, 0.0f, 8.0f, 0.0f)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SAVVMEditorToolkitOutputPathPicker::GetOutputPathText)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(12.f)
				.AutoWidth()
				[
					SNew(SButton)
					.ToolTipText(NSLOCTEXT("AVVMEditorToolkit", "BrowseTooltip", "Choose output directory"))
					.OnClicked(this, &SAVVMEditorToolkitOutputPathPicker::OnButtonClicked)
					.Content()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(8.0f, 0.0f, 8.0f, 0.0f)
						.AutoWidth()
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush("Icons.Folder"))
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(8.0f, 0.0f, 8.0f, 0.0f)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("AVVMEditorToolkit", "BrowseButton", "Browse..."))
						]
					]
				]
			]
		]
	];
}

void SAVVMEditorToolkitOutputPathPicker::OnOutputPathChanged(const FString& NewOutputPath)
{
	OutputPathText = FText::FromString(NewOutputPath);
	OnPathSelected.ExecuteIfBound(NewOutputPath);

	if (PathPickerWindow.IsValid())
	{
		PathPickerWindow->RequestDestroyWindow();
	}
}

FText SAVVMEditorToolkitOutputPathPicker::GetOutputPathText() const
{
	return OutputPathText.IsEmpty() ? NSLOCTEXT("AVVMEditorToolkit", "Output Path", "Select an output directory...") : OutputPathText;
}

FReply SAVVMEditorToolkitOutputPathPicker::OnMouseDoubleClicked(const FGeometry& Geometry,
                                                                const FPointerEvent& PointerEvent)
{
	return OnButtonClicked();
}

FReply SAVVMEditorToolkitOutputPathPicker::OnButtonClicked()
{
	TSharedPtr<SWidget> PathPicker{nullptr};
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FPathPickerConfig PathPickerConfig;
		PathPickerConfig.DefaultPath = TEXT("/Game");
		PathPickerConfig.OnPathSelected = FOnPathSelected::CreateRaw(this, &SAVVMEditorToolkitOutputPathPicker::OnOutputPathChanged);

		auto& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		PathPicker = ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig);

		// @gdemers reset output path
		OnOutputPathChanged(PathPickerConfig.DefaultPath);
	}

	if (!PathPicker.IsValid())
	{
		return FReply::Unhandled();
	}
	else
	{
		SAssignNew(PathPickerWindow, SWindow)
		.Title(NSLOCTEXT("AssetOutputPath", "WindowTile", "Choose Output Path"))
		.ClientSize(FVector2D(400.0f, 500.0f))
		.SizingRule(ESizingRule::UserSized)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.0f)
			[
				PathPicker->AsShared()
			]
		];

		FSlateApplication::Get().AddWindow(PathPickerWindow.ToSharedRef());
		return FReply::Handled();
	}
}
