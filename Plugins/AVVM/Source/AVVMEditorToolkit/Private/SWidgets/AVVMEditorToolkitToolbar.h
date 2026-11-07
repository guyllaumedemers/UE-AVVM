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
#include "Widgets/SCompoundWidget.h"

class FAVVMEditorToolkit_Core;
struct FAVVMBuilderInfo;
class FExtender;
class FUICommandInfo;
class UAVVMEditorToolkitBuilderObject;
class SHorizontalBox;
class UToolMenu;

DECLARE_DELEGATE_OneParam(FAVVMOnBuilderObjectChanged, const UAVVMEditorToolkitBuilderObject* BuilderObject);

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitToolbar is the main Slate element used for drawing our Tool toolbar. All menu entry generation
 *	is piped down through this Root.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitToolbar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitToolbar){}
	SLATE_EVENT(FAVVMOnBuilderObjectChanged, OnBuilderObjectChanged)
	SLATE_ATTRIBUTE(TSharedPtr<FAVVMEditorToolkit_Core>, Core)
	SLATE_ATTRIBUTE(TSharedPtr<FExtender>, MenuBarExtenders)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	void MakeGenericMenuEntry(const UAVVMEditorToolkitBuilderObject* BuilderObject, TSharedPtr<FAVVMEditorToolkit_Core> Core, TSharedPtr<FExtender> MenuBarExtenders);
	void OnBuilderObjectsAcquired(const UAVVMEditorToolkitBuilderObject* BuilderContextObject, TSharedPtr<FAVVMEditorToolkit_Core> Core, TSharedPtr<FExtender> MenuBarExtenders);

	TSharedPtr<SHorizontalBox> MenuBar{nullptr};
	FDelegateHandle Handle{};
};
