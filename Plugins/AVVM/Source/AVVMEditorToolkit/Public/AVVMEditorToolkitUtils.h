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

#include "AssetToolsModule.h"
#include "DataRegistryId.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWindow.h"

#include "AVVMEditorToolkitUtils.generated.h"

class UDataRegistry;
class UDataRegistrySource_DataTable;

/**
 *	Class description:
 *	
 *	FAVVMWindowContextArgs is a context struct encapsulating parameters to an SWindow initialization.
 */
struct AVVMEDITORTOOLKIT_API FAVVMWindowContextArgs
{
	FText Title{FText::GetEmpty()};
	TSharedPtr<SWidget> Content{};
	FRequestDestroyWindowOverride OnRequestDestroyWindowOverride{};
	FOnWindowClosed OnWindowClosed{};
};

/**
 *	Class description:
 *	
 *	FAVVMWindowContextArgs is a context struct encapsulating parameters to an SWindow initialization.
 */
struct AVVMEDITORTOOLKIT_API FAVVMAssetFactoryContextArgs
{
	FString AssetName{};
	FString PackagePath{};
	UFactory* AssetFactory{nullptr};
	UClass* AssetClass{nullptr};
};

/**
 *	Class description:
 *	
 *	UAVVMEditorToolkitUtils is a function library for reusable api.
 */
UCLASS()
class AVVMEDITORTOOLKIT_API UAVVMEditorToolkitUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TSharedRef<SWindow> OpenWindow(const FAVVMWindowContextArgs& Params);
	static TArray<const UDataRegistrySource_DataTable*> GetDataTables(const FDataRegistryType& RegistryType);
	static UDataRegistry* GetDataRegistry(const FDataRegistryType& RegistryType);

	template <typename TAsset>
	static TAsset* CreateOpenAsset(const FAVVMAssetFactoryContextArgs& Params);
};

template <typename TAsset>
TAsset* UAVVMEditorToolkitUtils::CreateOpenAsset(const FAVVMAssetFactoryContextArgs& Params)
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		UObject* NewAsset = AssetTools.Get().CreateAsset(Params.AssetName, Params.PackagePath, Params.AssetClass, Params.AssetFactory);
		AssetTools.Get().OpenEditorForAssets({NewAsset});
		return Cast<TAsset>(NewAsset);
	}
	else
	{
		return nullptr;
	}
}
