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

class FAVVMEditorToolkit_Core;

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitRoot is the main Slate element used for drawing our Tool. All sub-element generation
 *	is piped down through this Root.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitRoot : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitRoot) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 InTabIndex);
	void Setup(TSharedPtr<FAVVMEditorToolkit_Core> Core, TSharedPtr<FExtender> MenuBarExtenders);

protected:
	/** Pointer to the widget that houses the level editor's mode toolbar */
	TSharedPtr<SBorder> SecondaryMenuToolbarWidget;
};
