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

#ifdef UE_ENABLE_AVVM_EDITORTOOLKIT
#include "AVVMEditorToolkitBuilderObject.h"
#endif

#include "InventoryToolkitBuilderObject.generated.h"

class FUICommandInfo;

#if WITH_EDITORONLY_DATA

/**
 *	Class description:
 *	
 *	UInventoryToolkitBuilderObject is a UObject type used to build editor slate Tool used by
 *	the InventorySample plugin.
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryToolkitBuilderObject : public UAVVMEditorToolkitBuilderObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> BuildWidget() override;
	virtual void RegisterCommands(TSharedPtr<FAVVMEditorToolkit_Core> Core) override;

protected:
	virtual TArray<TSharedPtr<FUICommandInfo>> GetUICommands() const override;
	// @gdemers data table editor commands
	// Notes : it's expected that the following commands create a context object that's plugin specific
	// so RowTable actions are following rules specific plugin implementation details.
	TSharedPtr<FUICommandInfo> OpenDataTableEditor_RecentFiles = nullptr;
	TSharedPtr<FUICommandInfo> OpenDataTableEditor_OpenFile = nullptr;
	TSharedPtr<FUICommandInfo> OpenDataTableEditor_SaveAll = nullptr;
};

#endif
