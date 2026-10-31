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
#include "UObject/SavePackage.h"
#include "Widgets/SWindow.h"

#include "AVVMEditorToolkitUtils.generated.h"

class UAVVMEditorToolkitDataEditObject;
class UDataRegistry;
class UDataRegistrySource_DataTable;

/**
 *	@gdemers explicit directory for defined project structure,
 *	and asset generation output.
 */
#ifndef DATA_REGISTRY_DIRECTORY
#define DATA_REGISTRY_DIRECTORY TEXT("DataRegistries/")
#endif

#ifndef DATA_TABLE_DIRECTORY
#define DATA_TABLE_DIRECTORY TEXT("DataTables/")
#endif

#ifndef DATA_ASSET_DIRECTORY
#define DATA_ASSET_DIRECTORY TEXT("DataAssets/")
#endif

/**
 *	Class description:
 *	
 *	FAVVMEditorToolkitWindowContextArgs is a context struct encapsulating parameters to an SWindow initialization.
 */
struct AVVMEDITORTOOLKIT_API FAVVMEditorToolkitWindowContextArgs
{
	FText Title{FText::GetEmpty()};
	TSharedPtr<SWidget> Content{};
	FRequestDestroyWindowOverride OnRequestDestroyWindowOverride{};
	FOnWindowClosed OnWindowClosed{};
};

/**
 *	Class description:
 *	
 *	FAVVMEditorToolkitFactoryContextArgs is a context struct encapsulating parameters to an UFactory instance.
 */
struct AVVMEDITORTOOLKIT_API FAVVMEditorToolkitFactoryContextArgs
{
	FString AssetName{};
	FString PackagePath{};
	UFactory* AssetFactory{nullptr};
	UClass* AssetClass{nullptr};
};

/**
 *	Class description:
 *	
 *	FAVVMDataRegistryContextArgs is a context struct encapsulating parameters to an UDataRegistry UFactory.
 */
USTRUCT(BlueprintType)
struct AVVMEDITORTOOLKIT_API FAVVMEditorToolkitScriptStructContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	const UScriptStruct* ScriptStruct{nullptr};

	UPROPERTY(Transient, BlueprintReadWrite)
	FDataRegistryType RegistryType{};

	UPROPERTY(Transient, BlueprintReadWrite)
	FString PackagePath{};
};

/**
 *	Class description:
 *	
 *	FAVVMEditorDataTableOutputContextArgs is a context struct encapsulating parameters output asset at path.
 */
USTRUCT(BlueprintType)
struct AVVMEDITORTOOLKIT_API FAVVMEditorDataTableOutputContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UDataTable> DataTable{nullptr};

	UPROPERTY(Transient, BlueprintReadWrite)
	FName RowName{NAME_None};

	UPROPERTY(Transient, BlueprintReadWrite)
	FString PackagePath{};
};

/**
 *	Class description:
 *	
 *	FAVVMEditorDataTableActorIdentifierContextArgs is a context struct encapsulating parameters output for identifying.
 */
USTRUCT(BlueprintType)
struct AVVMEDITORTOOLKIT_API FAVVMEditorDataTableActorIdentifierContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const UAVVMEditorToolkitDataEditObject> EditObject{nullptr};

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UDataTable> DataTable{nullptr};

	UPROPERTY(Transient, BlueprintReadWrite)
	FName RowName{NAME_None};

	UPROPERTY(Transient, BlueprintReadWrite)
	FName ActorType{NAME_None};
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
	static TSharedRef<SWindow> OpenWindow(const FAVVMEditorToolkitWindowContextArgs& Params);
	static TArray<const UDataRegistrySource_DataTable*> GetDataTables(const FDataRegistryType& RegistryType);

	UFUNCTION(BlueprintCallable)
	static UDataRegistry* GetDataRegistry(const FDataRegistryType& RegistryType);
	static FString GetStandardizedName(const FString& PackagePath, FString&& AssetPrefix, FString&& AssetName);

	UFUNCTION(BlueprintCallable)
	static bool DoesDataRegistryTypeExist(const FDataRegistryType& RegistryType);

	UFUNCTION(BlueprintCallable)
	static UDataRegistry* CreateMissingDataRegistry(const FAVVMEditorToolkitScriptStructContextArgs& Params);

	UFUNCTION(BlueprintCallable)
	static bool AppendDataRegistryScanDirectories(const FString& DataRegistryPathName);

	UFUNCTION(BlueprintCallable)
	static UDataTable* CreateMissingDataTable(const FAVVMEditorToolkitScriptStructContextArgs& Params);

	UFUNCTION(BlueprintCallable)
	static UDataTable* GetDependentDataTable(const FDataRegistryType& RegistryType, const FString& TopLevelPath, const FString& OutputPackagePath);

	UFUNCTION(BlueprintCallable)
	static bool CreateActorIdentifierRow(const FAVVMEditorDataTableActorIdentifierContextArgs& Params);

	UFUNCTION(BlueprintCallable)
	static bool CreateActorDefinitionRow(const FAVVMEditorDataTableOutputContextArgs& Params);

	UFUNCTION(BlueprintCallable)
	static bool CreateActorUIDefinitionRow(const FAVVMEditorDataTableOutputContextArgs& Params);

	UFUNCTION(BlueprintCallable)
	static bool DoesActorDefinitionRowHaveData(const FString& NewPackagePath, const FName NewRowName, FTableRowBase& NewRow);

	UFUNCTION(BlueprintCallable)
	static bool DoesActorUIDefinitionRowHaveData(const FString& NewPackagePath, const FName NewRowName, FTableRowBase& NewRow);

	template <typename TAsset>
	static TAsset* CreateOpenAsset(const FAVVMEditorToolkitFactoryContextArgs& Params);
	
	UFUNCTION(BlueprintCallable)
	static FString CreateTopLevelDirectory(const FString& PackagePath, const FString& DirectoryName);
	
	UFUNCTION(BlueprintCallable)
	static bool SaveDirtyPackage(UObject* Asset);
	
	UFUNCTION(BlueprintCallable)
	static bool DeleteDataRegistryId(UDataTable* DataTable, const FDataRegistryId& RegistryId, const bool bShouldRemoveRowOnly = false);
};

template <typename TAsset>
TAsset* UAVVMEditorToolkitUtils::CreateOpenAsset(const FAVVMEditorToolkitFactoryContextArgs& Params)
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return nullptr;
	}

	auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetTools.Get().CreateAsset(Params.AssetName, Params.PackagePath, Params.AssetClass, Params.AssetFactory);
	if (!ensureAlwaysMsgf(IsValid(NewAsset), TEXT("Failed to create asset.")))
	{
		return nullptr;
	}

	const bool bSuccess = UAVVMEditorToolkitUtils::SaveDirtyPackage(NewAsset);
	if (ensureAlwaysMsgf(bSuccess, TEXT("Failed to save")))
	{
		AssetTools.Get().OpenEditorForAssets({NewAsset});
	}

	return Cast<TAsset>(NewAsset);
}
