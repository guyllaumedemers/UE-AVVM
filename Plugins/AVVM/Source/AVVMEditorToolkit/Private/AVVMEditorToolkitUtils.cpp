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
#include "AVVMEditorToolkitUtils.h"

#include "AVVMEditorToolkitModule.h"
#include "DataRegistrySettings.h"
#include "DataRegistrySource_DataTable.h"
#include "DataRegistrySubsystem.h"
#include "DataTableEditorUtils.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/AVVMActorUIDefinitionDataAsset.h"
#include "Data/AVVMDataRegistry.h"
#include "Engine/DataTable.h"
#include "Factories/DataAssetFactory.h"
#include "Factories/DataTableFactory.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "SWidgets/AVVMEditorToolkitDataEditor.h"
#include "UObject/AssetRegistryTagsContext.h"

TSharedRef<SWindow> UAVVMEditorToolkitUtils::OpenWindow(const FAVVMEditorToolkitWindowContextArgs& Params)
{
	FDisplayMetrics DisplayMetrics{};
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayMetrics.PrimaryDisplayWorkAreaRect.Left, DisplayMetrics.PrimaryDisplayWorkAreaRect.Top);
	const FVector2D ClientSize(1200.0f * DPIScaleFactor, 800.0f * DPIScaleFactor);

	auto NewWindow = SNew(SWindow)
		.Title(Params.Title)
		.CreateTitleBar(true)
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		.IsInitiallyMaximized(false)
		.IsInitiallyMinimized(false)
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(ClientSize)
		.AdjustInitialSizeAndPositionForDPIScale(false)
		.Content()
		[
			Params.Content.ToSharedRef()
		];

	NewWindow->SetRequestDestroyWindowOverride(Params.OnRequestDestroyWindowOverride);
	NewWindow->SetOnWindowClosed(Params.OnWindowClosed);

	FSlateApplication::Get().AddWindow(NewWindow, true);
	FGlobalTabmanager::Get()->SetAllowWindowMenuBar(true);
	// @gdemers IMPORTANT - Do not update the Window Root, otherwise closure events from calling parent
	// will affect any subsequent (child) window opened following root change.
	// FGlobalTabmanager::Get()->SetRootWindow(NewWindow);
	// FSlateNotificationManager::Get().SetRootWindow(NewWindow);
	return NewWindow;
}

TArray<const UDataRegistrySource_DataTable*> UAVVMEditorToolkitUtils::GetDataTables(const FDataRegistryType& RegistryType)
{
	TArray<const UDataRegistrySource_DataTable*> OutResult{};

	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return OutResult;
	}

	// @gdemers Data Table newly added cannot refresh due to being empty. We should still be able to lookup from DataSource, but the api is protected
	// which is why we will enforce using our custom derived type, and access protected members by friending the derived class.
	auto* DataRegistry = Cast<UAVVMDataRegistry>(Subsystem->GetRegistryForType(RegistryType));
	if (!IsValid(DataRegistry))
	{
		return OutResult;
	}

	DataRegistry->RefreshRuntimeSources();
	for (const auto& Element : DataRegistry->RuntimeSources)
	{
		const auto* Source = Cast<UDataRegistrySource_DataTable>(Element);
		if (IsValid(Source))
		{
			OutResult.AddUnique(Source);
		}
	}

	return OutResult;
}

UDataRegistry* UAVVMEditorToolkitUtils::GetDataRegistry(const FDataRegistryType& RegistryType)
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	return IsValid(Subsystem) ? Subsystem->GetRegistryForType(RegistryType) : nullptr;
}

FString UAVVMEditorToolkitUtils::GetStandardizedName(const FString& PackagePath,
                                                     FString&& AssetPrefix,
                                                     FString&& AssetName)
{
	FString PluginPath{TEXT("InventorySampleEditor")};

	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPluginFromPath(PackagePath);
	if (Plugin.IsValid())
	{
		PluginPath = Plugin->GetName();
	}

	return FString::Printf(TEXT("%s_%s_%s_%s"),
	                       *AssetPrefix,
	                       PROJECT_ASSET_PREFIX,
	                       *PluginPath,
	                       *AssetName);
}

bool UAVVMEditorToolkitUtils::DoesDataRegistryTypeExist(const FDataRegistryType& RegistryType)
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	return IsValid(Subsystem) ? IsValid(Subsystem->GetRegistryForType(RegistryType)) : false;
}

UDataRegistry* UAVVMEditorToolkitUtils::CreateMissingDataRegistry(const FAVVMEditorToolkitScriptStructContextArgs& Params)
{
	auto* Factory = NewObject<UAVVMDataRegistryFactory>();
	if (!IsValid(Factory))
	{
		return nullptr;
	}

	Factory->DataRegistryClass = UAVVMDataRegistry::StaticClass();
	Factory->ItemStruct = Params.ScriptStruct;
	Factory->RegistryType = Params.RegistryType;

	FAVVMEditorToolkitFactoryContextArgs NewParams{};
	NewParams.AssetName = UAVVMEditorToolkitUtils::GetStandardizedName(Params.PackagePath, TEXT("DR"), *GetNameSafe(Factory->ItemStruct));
	NewParams.PackagePath = Params.PackagePath;
	NewParams.AssetFactory = Factory;
	NewParams.AssetClass = UAVVMDataRegistry::StaticClass();
	
	auto* DataRegistry = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMDataRegistry>(NewParams);
	if (!ensureAlwaysMsgf(IsValid(DataRegistry), TEXT("Failed to create new DataRegistry.")))
	{
		return nullptr;
	}

	const bool bResult = UAVVMEditorToolkitUtils::AppendDataRegistryScanDirectories(DataRegistry->GetPathName());
	if (!ensureAlwaysMsgf(bResult, TEXT("Failed to append DataRegistrySettings scan directories.")))
	{
		return DataRegistry;
	}

	auto* MetaSource_DataTable = NewObject<UMetaDataRegistrySource_DataTable>(DataRegistry);
	if (IsValid(MetaSource_DataTable))
	{
		FAssetManagerSearchRules SearchRules{};
		SearchRules.AssetScanPaths = {TEXT("")}; // search all
		SearchRules.AssetBaseClass = UDataTable::StaticClass();

		MetaSource_DataTable->TableRules = FDataRegistrySource_DataTableRules{true};
		MetaSource_DataTable->AssetUsage = EMetaDataRegistrySourceAssetUsage::SearchAndRegisterAssets;
		MetaSource_DataTable->SearchRules = MoveTemp(SearchRules);
		DataRegistry->DataSources.Add(MetaSource_DataTable);
	}

	// @gdemers handle refreshing registry, and data table visibility.
	DataRegistry->EditorRefreshRegistry();
	return DataRegistry;
}

bool UAVVMEditorToolkitUtils::AppendDataRegistryScanDirectories(const FString& DataRegistryPathName)
{
	auto* MutableSettings = GetMutableDefault<UDataRegistrySettings>();
	if (!ensureAlwaysMsgf(IsValid(MutableSettings), TEXT("Failed to add the missing DataRegistry Path")))
	{
		return false;
	}

	const auto* SearchResult = MutableSettings->DirectoriesToScan.FindByPredicate([&DataRegistryPathName](const FDirectoryPath& DirPath)
	{
		return FPaths::IsUnderDirectory(DirPath.Path, DataRegistryPathName);
	});

	if (SearchResult != nullptr)
	{
		// @gdemers path already exist which isnt considered failure.
		return true;
	}
	
	FStringView OutRoot{};
	FPackageName::SplitPackageName(DataRegistryPathName, &OutRoot, nullptr, nullptr, FPackageName::EPathFormatFlags::MountPointLeadingSlash);
	MutableSettings->DirectoriesToScan.Add(FDirectoryPath{FString{OutRoot}});
	ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(MutableSettings), TEXT("Failed to save modified Settings."));

	FFormatNamedArguments Args{};
	Args.Add(TEXT("Directory"), FText::FromStringView(OutRoot));

	FMessageDialog::Open(EAppMsgCategory::Warning,
						 EAppMsgType::Ok,
						 FText::Format(NSLOCTEXT("AVVMEditorToolkit", "NotifyDataRegistryAddedAtPAth", "Adding a new Top-level directory {Directory} to the DataRegistry Settings."), Args));

	auto* RegistrySubsystem = UDataRegistrySubsystem::Get();
	if (IsValid(RegistrySubsystem))
	{
		RegistrySubsystem->ReinitializeFromConfig();
	}

	return true;
}

UDataTable* UAVVMEditorToolkitUtils::CreateMissingDataTable(const FAVVMEditorToolkitScriptStructContextArgs& Params)
{
	auto* Factory = NewObject<UDataTableFactory>();
	if (!IsValid(Factory))
	{
		return nullptr;
	}

	Factory->Struct = Params.ScriptStruct;

	FAVVMEditorToolkitFactoryContextArgs NewParams{};
	NewParams.AssetName = UAVVMEditorToolkitUtils::GetStandardizedName(Params.PackagePath, TEXT("DT"), *GetNameSafe(Factory->Struct));
	NewParams.PackagePath = Params.PackagePath;
	NewParams.AssetFactory = Factory;
	NewParams.AssetClass = UDataTable::StaticClass();

	auto* DataTable = UAVVMEditorToolkitUtils::CreateOpenAsset<UDataTable>(NewParams);
	if (!ensureAlwaysMsgf(IsValid(DataTable), TEXT("Failed to create new DataTable.")))
	{
		return nullptr;
	}

	auto* RegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(RegistrySubsystem))
	{
		return DataTable;
	}

	FAssetData Asset{DataTable, FAssetData::ECreationFlags::None, EAssetRegistryTagsCaller::AssetRegistryLoad};
	RegistrySubsystem->RegisterSpecificAsset(Params.RegistryType, Asset, 0);

	auto* DataRegistry = RegistrySubsystem->GetRegistryForType(Params.RegistryType);
	if (ensureAlwaysMsgf(IsValid(DataRegistry), TEXT("Failed to retrieve valid DataRegistry.")))
	{
		// @gdemers handle refreshing registry, and data table visibility.
		DataRegistry->EditorRefreshRegistry();
	}

	return DataTable;
}

UDataTable* UAVVMEditorToolkitUtils::GetDependentDataTable(const FDataRegistryType& RegistryType,
                                                           const FString& TopLevelPath)
{
	// TODO @gdemers search for DT under this directory
	return nullptr;
}

bool UAVVMEditorToolkitUtils::CreateActorIdentifierRow(const FAVVMEditorDataTableActorIdentifierContextArgs& Params)
{
	bool bResult{false};
	if (!Params.EditObject.IsValid())
	{
		return bResult;
	}

	auto* Row = reinterpret_cast<FAVVMActorIdentifierDataTableRow*>(FDataTableEditorUtils::AddRow(Params.DataTable.Get(), Params.RowName));
	if (ensureAlwaysMsgf(Row != nullptr, TEXT("Failed to cast Row Type.")))
	{
		Row->UniqueId = Params.EditObject->GenerateActorIdentifier(Params.ActorType);
		bResult = (Row->UniqueId != INDEX_NONE);
		ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(Params.DataTable.Get()), TEXT("Failed to save Package."));
	}

	return bResult;
}

bool UAVVMEditorToolkitUtils::CreateActorDefinitionRow(const FAVVMEditorDataTableOutputContextArgs& Params)
{
	bool bResult{false};

	auto* Row = reinterpret_cast<FAVVMActorDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(Params.DataTable.Get(), Params.RowName));
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Failed to cast Row Type.")))
	{
		return bResult;
	}

	auto* DA_Factory = NewObject<UDataAssetFactory>();
	if (!IsValid(DA_Factory))
	{
		return bResult;
	}

	DA_Factory->DataAssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

	FAVVMEditorToolkitFactoryContextArgs NewParams{};
	NewParams.AssetName = Params.RowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
	NewParams.PackagePath = Params.PackagePath; // share same package path as Blueprint asset
	NewParams.AssetFactory = DA_Factory;
	NewParams.AssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

	auto* ActorDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorDefinitionDataAsset>(NewParams);
	if (ensureAlwaysMsgf(IsValid(ActorDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
	{
		Row->ActorDefinition = ActorDefinitionDataAsset;
		bResult = (false == Row->ActorDefinition.IsNull());
		ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(Params.DataTable.Get()), TEXT("Failed to save Package."));
	}

	return bResult;
}

bool UAVVMEditorToolkitUtils::CreateActorUIDefinitionRow(const FAVVMEditorDataTableOutputContextArgs& Params)
{
	bool bResult{false};

	auto* Row = reinterpret_cast<FAVVMActorUIDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(Params.DataTable.Get(), Params.RowName));
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Failed to cast Row Type.")))
	{
		return bResult;
	}

	auto* DA_Factory = NewObject<UDataAssetFactory>();
	if (!IsValid(DA_Factory))
	{
		return false;
	}

	DA_Factory->DataAssetClass = UAVVMActorUIDefinitionDataAsset::StaticClass();

	FAVVMEditorToolkitFactoryContextArgs NewParams{};
	NewParams.AssetName = Params.RowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
	NewParams.PackagePath = Params.PackagePath; // share same package path as Blueprint asset
	NewParams.AssetFactory = DA_Factory;
	NewParams.AssetClass = UAVVMActorUIDefinitionDataAsset::StaticClass();

	auto* ActorUIDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorUIDefinitionDataAsset>(NewParams);
	if (ensureAlwaysMsgf(IsValid(ActorUIDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
	{
		Row->ActorUIDefinition = ActorUIDefinitionDataAsset;
		bResult = (false == Row->ActorUIDefinition.IsNull());
		ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(Params.DataTable.Get()), TEXT("Failed to save Package."));
	}

	return bResult;
}

bool UAVVMEditorToolkitUtils::DoesActorDefinitionRowHaveData(const FString& NewPackagePath,
                                                             const FName NewRowName,
                                                             FTableRowBase& NewRow)
{
	auto* Row = StaticCast<FAVVMActorDefinitionDataTableRow*>(&NewRow);
	if (Row == nullptr)
	{
		return false;
	}

	// @gdemers we expect a non-null entry to NOT trigger a creation of a new row entry.
	if (!Row->ActorDefinition.IsNull())
	{
		return true;
	}

	auto* DA_Factory = NewObject<UDataAssetFactory>();
	if (!IsValid(DA_Factory))
	{
		return false;
	}

	DA_Factory->DataAssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

	FAVVMEditorToolkitFactoryContextArgs NewParams{};
	NewParams.AssetName = NewRowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
	NewParams.PackagePath = NewPackagePath; // share same package path as Blueprint asset
	NewParams.AssetFactory = DA_Factory;
	NewParams.AssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

	auto* ActorDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorDefinitionDataAsset>(NewParams);
	if (ensureAlwaysMsgf(IsValid(ActorDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
	{
		Row->ActorDefinition = ActorDefinitionDataAsset;
		ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(ActorDefinitionDataAsset), TEXT("Failed to save Package."));
	}

	return true;
}

bool UAVVMEditorToolkitUtils::DoesActorUIDefinitionRowHaveData(const FString& NewPackagePath,
                                                               const FName NewRowName,
                                                               FTableRowBase& NewRow)
{
	auto* Row = StaticCast<FAVVMActorUIDefinitionDataTableRow*>(&NewRow);
	if (Row == nullptr)
	{
		return false;
	}

	// @gdemers we expect a non-null entry to NOT trigger a creation of a new row entry.
	if (!Row->ActorUIDefinition.IsNull())
	{
		return true;
	}

	auto* DA_Factory = NewObject<UDataAssetFactory>();
	if (!IsValid(DA_Factory))
	{
		return false;
	}

	DA_Factory->DataAssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

	FAVVMEditorToolkitFactoryContextArgs NewParams{};
	NewParams.AssetName = NewRowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
	NewParams.PackagePath = NewPackagePath; // share same package path as Blueprint asset
	NewParams.AssetFactory = DA_Factory;
	NewParams.AssetClass = UAVVMActorUIDefinitionDataAsset::StaticClass();

	auto* ActorUIDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorUIDefinitionDataAsset>(NewParams);
	if (ensureAlwaysMsgf(IsValid(ActorUIDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
	{
		Row->ActorUIDefinition = ActorUIDefinitionDataAsset;
		ensureAlwaysMsgf(UAVVMEditorToolkitUtils::SaveDirtyPackage(ActorUIDefinitionDataAsset), TEXT("Failed to save Package."));
	}

	return true;
}

bool UAVVMEditorToolkitUtils::SaveDirtyPackage(UObject* Asset)
{
	UPackage* Package = IsValid(Asset) ? Asset->GetPackage() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(Package), TEXT("Failed to retrieve package.")))
	{
		return false;
	}

	FSavePackageArgs SavePackageArgs{};
	SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SavePackageArgs.SaveFlags = SAVE_FromAutosave;
	SavePackageArgs.Error = GLog;

	const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
	return UPackage::SavePackage(Package, Asset, *PackageFileName, MoveTemp(SavePackageArgs));
}
