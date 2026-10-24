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
#include "SWidgets/AVVMEditorToolkitDataEditor.h"

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

	// @gdemers ensure we always get latest state.
	Subsystem->RefreshRegistryMap();

	const auto* DataRegistry = Subsystem->GetRegistryForType(RegistryType);
	if (!IsValid(DataRegistry))
	{
		return OutResult;
	}

	// @gdemers sadly, the DataRegistry class doesnt allow direct access to the DataSources property.
	TArray<FDataRegistrySourceItemId> OutElements{};
	DataRegistry->GetAllSourceItems(OutElements);

	for (const auto& Element : OutElements)
	{
		const auto* Source = Cast<UDataRegistrySource_DataTable>(Element.CachedSource);
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

UDataRegistry* UAVVMEditorToolkitUtils::CreateMissingDataRegistry(const FAVVMEditorToolkitDataRegistryContextArgs& Params)
{
	auto* Factory = NewObject<UAVVMDataRegistryFactory>();
	if (IsValid(Factory))
	{
		Factory->DataRegistryClass = UAVVMDataRegistry::StaticClass();
		Factory->ItemStruct = Params.ScriptStruct;
		Factory->RegistryType = Params.RegistryType;

		FAVVMEditorToolkitFactoryContextArgs NewParams{};
		NewParams.AssetName = UAVVMEditorToolkitUtils::GetStandardizedName(Params.PackagePath, TEXT("DR"), *GetNameSafe(Factory->ItemStruct));
		NewParams.PackagePath = Params.PackagePath;
		NewParams.AssetFactory = Factory;
		NewParams.AssetClass = UAVVMDataRegistry::StaticClass();
		return UAVVMEditorToolkitUtils::CreateOpenAsset<UDataRegistry>(NewParams);
	}
	else
	{
		return nullptr;
	}
}

UDataTable* UAVVMEditorToolkitUtils::CreateMissingDataTable(const FAVVMEditorToolkitDataTableContextArgs& Params)
{
	auto* Factory = NewObject<UDataTableFactory>();
	if (IsValid(Factory))
	{
		Factory->Struct = Params.ScriptStruct;

		FAVVMEditorToolkitFactoryContextArgs Params{};
		Params.AssetName = UAVVMEditorToolkitUtils::GetStandardizedName(Params.PackagePath, TEXT("DT"), *GetNameSafe(Factory->Struct));
		Params.PackagePath = Params.PackagePath;
		Params.AssetFactory = Factory;
		Params.AssetClass = UDataTable::StaticClass();
		return UAVVMEditorToolkitUtils::CreateOpenAsset<UDataTable>(Params);
	}
	else
	{
		return nullptr;
	}
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
	}

	return bResult;
}

bool UAVVMEditorToolkitUtils::CreateActorDefinitionRow(const FAVVMEditorDataTableOutputContextArgs& Params)
{
	bool bResult{false};

	auto* Row = reinterpret_cast<FAVVMActorDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(Params.DataTable.Get(), Params.RowName));
	if (ensureAlwaysMsgf(Row != nullptr, TEXT("Failed to cast Row Type.")))
	{
		auto* DA_Factory = NewObject<UDataAssetFactory>();
		if (!IsValid(DA_Factory))
		{
			return bResult;
		}

		DA_Factory->DataAssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

		FAVVMEditorToolkitFactoryContextArgs DA_Params{};
		DA_Params.AssetName = Params.RowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
		DA_Params.PackagePath = Params.PackagePath; // share same package path as Blueprint asset
		DA_Params.AssetFactory = DA_Factory;
		DA_Params.AssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

		auto* ActorDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorDefinitionDataAsset>(DA_Params);
		if (ensureAlwaysMsgf(IsValid(ActorDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
		{
			Row->ActorDefinition = ActorDefinitionDataAsset;
			bResult = (false == Row->ActorDefinition.IsNull());
		}
	}

	return bResult;
}

bool UAVVMEditorToolkitUtils::CreateActorUIDefinitionRow(const FAVVMEditorDataTableOutputContextArgs& Params)
{
	bool bResult{false};

	auto* Row = reinterpret_cast<FAVVMActorUIDefinitionDataTableRow*>(FDataTableEditorUtils::AddRow(Params.DataTable.Get(), Params.RowName));
	if (ensureAlwaysMsgf(Row != nullptr, TEXT("Failed to cast Row Type.")))
	{
		auto* DA_Factory = NewObject<UDataAssetFactory>();
		if (!IsValid(DA_Factory))
		{
			return false;
		}

		DA_Factory->DataAssetClass = UAVVMActorUIDefinitionDataAsset::StaticClass();

		FAVVMEditorToolkitFactoryContextArgs DA_Params{};
		DA_Params.AssetName = Params.RowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
		DA_Params.PackagePath = Params.PackagePath; // share same package path as Blueprint asset
		DA_Params.AssetFactory = DA_Factory;
		DA_Params.AssetClass = UAVVMActorUIDefinitionDataAsset::StaticClass();

		auto* ActorUIDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorUIDefinitionDataAsset>(DA_Params);
		if (ensureAlwaysMsgf(IsValid(ActorUIDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
		{
			Row->ActorUIDefinition = ActorUIDefinitionDataAsset;
			bResult = (false == Row->ActorUIDefinition.IsNull());
		}
	}

	return bResult;
}

bool UAVVMEditorToolkitUtils::EditMutableActorIdentifierRow(FTableRowBase& NewRow, const FName NewRowName)
{
	return true;
}

bool UAVVMEditorToolkitUtils::EditMutableActorDefinitionRow(FTableRowBase& NewRow, const FName NewRowName, const FString& NewPackagePath)
{
	auto* Row = StaticCast<FAVVMActorDefinitionDataTableRow*>(&NewRow);
	if (Row == nullptr)
	{
		return false;
	}

	if (!Row->ActorDefinition.IsNull())
	{
		// @gdemers we expect a non-null entry to NOT trigger a creation of a new row entry.
		return true;
	}

	auto* DA_Factory = NewObject<UDataAssetFactory>();
	if (IsValid(DA_Factory))
	{
		DA_Factory->DataAssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

		FAVVMEditorToolkitFactoryContextArgs DA_Params{};
		DA_Params.AssetName = NewRowName.ToString().Replace(TEXT("BP"), TEXT("DA"));
		DA_Params.PackagePath = NewPackagePath; // share same package path as Blueprint asset
		DA_Params.AssetFactory = DA_Factory;
		DA_Params.AssetClass = UAVVMActorDefinitionDataAsset::StaticClass();

		auto* ActorDefinitionDataAsset = UAVVMEditorToolkitUtils::CreateOpenAsset<UAVVMActorDefinitionDataAsset>(DA_Params);
		if (ensureAlwaysMsgf(IsValid(ActorDefinitionDataAsset), TEXT("Failed to create New Data Asset.")))
		{
			Row->ActorDefinition = ActorDefinitionDataAsset;
		}
	}

	return true;
}

bool UAVVMEditorToolkitUtils::EditMutableActorUIDefinitionRow(FTableRowBase& NewRow, const FName NewRowName, const FString& NewPackagePath)
{
	return true;
}
