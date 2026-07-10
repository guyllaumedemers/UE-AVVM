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
#include "AVVMGameModeAdditive.h"

#include "AVVMGameplayModule.h"
#include "AVVMGameplaySettings.h"
#include "AVVMLogger.h"
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"
#include "Misc/CommandLine.h"
#include "Templates/SubclassOf.h"

#if WITH_EDITOR
EDataValidationResult FAVVMGameModeAdditiveDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (GameModeAdditiveClass.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMGameModeAdditiveDefinitionDataTableRow", "", "Invalid SoftClassPtr."));
	}

	return Result;
}
#endif

const TArray<FString> UAVVMGameModeAdditiveUtils::ParseCmdOptions(const FString& GameModeOptions)
{
	static const TCHAR* Delims[] = {TEXT(","), TEXT(";"), TEXT(" ")};
	static const TCHAR* OptionFlag = TEXT("GameModeOptions=");
	static constexpr size_t Size = (sizeof(Delims) / sizeof(TCHAR*));

	FString UnsplitOptions = TEXT("");

	if (!FParse::Value(FCommandLine::Get(), OptionFlag, UnsplitOptions, false) &&
		!FParse::Value(*GameModeOptions, OptionFlag, UnsplitOptions, false))
	{
		return {};
	}

	TArray<FString> OutSplitOptions;
	UnsplitOptions.ParseIntoArray(OutSplitOptions, Delims, Size, true);

	return OutSplitOptions;
}

TMap<FName, UAVVMGameModeAdditive*> UAVVMGameModeAdditiveUtils::LoadSynchronous(const TArray<FString>& SplitOptions,
                                                                                UObject* Outer,
                                                                                TArray<FString>& OutFailedOrPluginSpecificOptions)
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		OutFailedOrPluginSpecificOptions = SplitOptions;
		return {};
	}

	// @gdemers Using UDataRegistrySubsystem allow for cross dll referencing of user defined
	// UAVVMGameModeAdditive (for example : when creating a BP in a GFP).
	const UDataRegistry* DataRegistry = Subsystem->GetRegistryForType(UAVVMGameplaySettings::GetGameModeAdditiveRegistryType());
	if (!IsValid(DataRegistry))
	{
		OutFailedOrPluginSpecificOptions = SplitOptions;
		return {};
	}

	TArray<const FAVVMGameModeAdditiveDefinitionDataTableRow*> OutGameModeAdditiveDataTableRows;
	DataRegistry->GetAllItems(TEXT(""), OutGameModeAdditiveDataTableRows);

	TArray<TSoftClassPtr<UAVVMGameModeAdditive>> OutGameModeAdditives;
	OutGameModeAdditives.Reserve(OutGameModeAdditiveDataTableRows.Num());
	
	for (const auto* TableRow : OutGameModeAdditiveDataTableRows)
	{
		if (TableRow != nullptr)
		{
			OutGameModeAdditives.Add(TableRow->GameModeAdditiveClass);
		}
	}

	TMap<FName, TSubclassOf<UAVVMGameModeAdditive>> OutClasses;
	for (const FString& SplitOption : SplitOptions)
	{
		const TSoftClassPtr<UAVVMGameModeAdditive> GameModeAdditiveSoftClass = UAVVMGameModeAdditiveUtils::GetGameModeAdditiveSoftClass(OutGameModeAdditives, SplitOption/*Cmdline Flag name used*/);
		if (!GameModeAdditiveSoftClass.IsValid())
		{
			// @gdemers we need to track which cmd line parameter werent able to resolved to handle the case
			// where GFP specific UAVVMGameModeAdditive are requested for load.
			OutFailedOrPluginSpecificOptions.Add(SplitOption);
			continue;
		}

		// @gdemers THIS IS REQUIRED TO BE SYNCHRONOUS, otherwise the UAVVMGameModeAdditive loaded from from AGameMode prior to Match
		const TSubclassOf<UAVVMGameModeAdditive> GameModeAdditiveClass = GameModeAdditiveSoftClass.LoadSynchronous();
		if (IsValid(GameModeAdditiveClass))
		{
			OutClasses.Add(FName(SplitOption), GameModeAdditiveClass);
		}
	}

	TMap<FName, UAVVMGameModeAdditive*> OutResults;
	for (const auto& [FName, GameModeAdditiveClass] : OutClasses)
	{
		auto* NewGameModeAdditive = NewObject<UAVVMGameModeAdditive>(Outer, GameModeAdditiveClass);
		OutResults.Add(FName, NewGameModeAdditive);
	}

	return OutResults;
}

TSoftClassPtr<UAVVMGameModeAdditive> UAVVMGameModeAdditiveUtils::GetGameModeAdditiveSoftClass(const TArray<TSoftClassPtr<UAVVMGameModeAdditive>>& OutGameModeAdditives,
                                                                                              const FString& CmdLineFlagName)
{
	const TSoftClassPtr<UAVVMGameModeAdditive>* NewGameModeAdditives = OutGameModeAdditives
			.FindByPredicate([Search = CmdLineFlagName](const TSoftClassPtr<UAVVMGameModeAdditive>& GameModeAdditiveClass)
			{
				if (GameModeAdditiveClass.IsNull())
				{
					return false;
				}

				// @gdemers its expected that the package name follow specific standard. i.e BP_{ProjectName}_AssetName
				// By doing so, we are able to parse cmd line parameter flags, and compare against the associated AssetPath of each class.
				const FTopLevelAssetPath AssetPath = GameModeAdditiveClass->GetClassPathName();

				TArray<FString> OutSplits;
				AssetPath.GetAssetName().ToString().ParseIntoArray(OutSplits, TEXT("_"));
				const FString ParsedPath = (OutSplits.Num() > 1) ? OutSplits[OutSplits.Num() - 2/*ignore _C extension in last entry*/] : TEXT("");
				return ParsedPath.Equals(Search);
			});

	if (ensureAlwaysMsgf(NewGameModeAdditives != nullptr,
	                     TEXT("Couldnt find %s in the available listing of UGameModeAdditive UObject references."),
	                     *CmdLineFlagName))
	{
		return *NewGameModeAdditives;
	}
	else
	{
		return nullptr;
	}
}
