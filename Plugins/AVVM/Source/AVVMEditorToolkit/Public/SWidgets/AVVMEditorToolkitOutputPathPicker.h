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
#pragma once

#include "CoreMinimal.h"

#include "AVVMEditorToolkitDataImporter.h"
#include "ContentBrowserDelegates.h"

#include "Widgets/SCompoundWidget.h"

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitOutputPathPicker is a slate context displaying information about a selected output path.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitOutputPathPicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitOutputPathPicker){}
	SLATE_EVENT(FOnPathSelected, OnPathSelected)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	void OnOutputPathChanged(const FString& NewOutputPath);
	FText GetOutputPathText() const;
	FReply OnMouseDoubleClicked(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	FReply OnButtonClicked();
	
	TSharedPtr<SWindow> PathPickerWindow{nullptr};
	FText OutputPathText{FText::GetEmpty()};
	FOnPathSelected OnPathSelected{};
};
