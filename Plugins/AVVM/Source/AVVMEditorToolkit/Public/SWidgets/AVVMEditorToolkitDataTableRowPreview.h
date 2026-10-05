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

#include "DataRegistryId.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitDataTableRowPreview is a slate context previewing Row Data.
 *	
 *	IMPORTANT : This is not to allow editing the actual Data Table Row entry, but rather display a preview version
 *	of the target entry.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitDataTableRowPreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitDataTableRowPreview){};
	SLATE_ATTRIBUTE(FDataRegistryId, RegistryId)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
private:
	TArray<UObject*> GetAssetsFromRegistryId() const;
	
	TSharedPtr<IDetailsView> ObjectPropertyView{nullptr};
	FDataRegistryId RegistryId{};
};
