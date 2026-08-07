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

#include "AVVMGameMode.h"
#include "AVVMGameplayModule.h"
#include "AVVMGameplaySettings.h"
#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Templates/SubclassOf.h"

bool UAVVMPredicateTask::WaitUntilDone_Implementation(FAVVMPredicateTaskResult& OutResult) const
{
	const double Now = UAVVMToolkitUtils::GetServerWorldTime(this);
	if (FMath::IsNearlyZero(OutResult.StartTimestamp))
	{
		OutResult.StartTimestamp = Now;
		NotifyStart();
	}

	const bool bIsWaiting = ((Now - OutResult.StartTimestamp) < Timeout);
	if (!bIsWaiting)
	{
		NotifyEnd();
		OutResult.StartTimestamp = 0.f;
		OutResult.CurrTaskIndex = FMath::Clamp(OutResult.CurrTaskIndex + 1, 0, INT32_MAX);
	}

	return bIsWaiting;
}

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
		if (GameModeAdditiveSoftClass.IsNull())
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
			OutClasses.Add(FName(GameModeAdditiveSoftClass.GetAssetName()), GameModeAdditiveClass);
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

				TArray<FString> OutSplits;
				GameModeAdditiveClass.GetAssetName().ParseIntoArray(OutSplits, TEXT("_"));
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

void UAVVMGameModeAdditiveUtils::AddOrRemoveGameModeAdditive(const UObject* WorldContextObject,
                                                             const FName& GameModeAdditiveClassAssetName,
                                                             const bool bAddOrRemove)
{
	auto* GameMode = Cast<AAVVMGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!IsValid(GameMode))
	{
		return;
	}

	const bool bDoesContains = GameMode->RuntimeGameModeAdditives.Contains(GameModeAdditiveClassAssetName);
	if (bDoesContains && bAddOrRemove)
	{
		return;
	}

	if (bAddOrRemove)
	{
		// @gdemers we dont parse the name here as we are using the AssetPath name as KVP
		// when caching the ptr on the AVVMGameMode.
		TArray<FString> OutSplits;
		GameModeAdditiveClassAssetName.ToString().ParseIntoArray(OutSplits, TEXT("_"));
		const FString ParsedPath = (OutSplits.Num() > 1) ? OutSplits[OutSplits.Num() - 2/*ignore _C extension in last entry*/] : TEXT("");
		
		TArray<FString> OutFailedOptions;

		const TArray<FString> SplitOptions = UAVVMGameModeAdditiveUtils::ParseCmdOptions(FString::Printf(TEXT("GameModeOptions=%s"), *ParsedPath));
		const TMap<FName, UAVVMGameModeAdditive*> GameModeAdditives = UAVVMGameModeAdditiveUtils::LoadSynchronous(SplitOptions, GameMode, OutFailedOptions);

		for (const auto& [Key, Value] : GameModeAdditives)
		{
			GameMode->RuntimeGameModeAdditives.Add(GameModeAdditiveClassAssetName, Value);

			AVVM_LOGGER_LOG(LogGameplay,
			                GameMode,
			                GameMode,
			                TEXT("Creating new GameModeAdditive %s."),
			                *Key.ToString());

			if (IsValid(Value))
			{
				Value->StartMatch();
			}
		}
	}
	else
	{
		TObjectPtr<UAVVMGameModeAdditive>& GameModeAdditive = GameMode->RuntimeGameModeAdditives[GameModeAdditiveClassAssetName];
		if (IsValid(GameModeAdditive))
		{
			GameModeAdditive->EndMatch();
		}

		AVVM_LOGGER_LOG(LogGameplay,
		                GameMode,
		                GameMode,
		                TEXT("Destroying GameModeAdditive %s."),
		                *GameModeAdditiveClassAssetName.ToString());

		GameMode->RuntimeGameModeAdditives.Remove(GameModeAdditiveClassAssetName);
	}
}

bool UAVVMGameModeAdditiveUtils::WaitUntilDone(const TArray<UAVVMPredicateTask*>& PredicateTaskCDOs,
                                               FAVVMPredicateTaskResult& OutResult)
{
	if (PredicateTaskCDOs.IsEmpty())
	{
		return false;
	}

	const bool bIsIndexValid = PredicateTaskCDOs.IsValidIndex(OutResult.CurrTaskIndex);
	if (!bIsIndexValid)
	{
		return false;
	}

	const UAVVMPredicateTask* PredicateTask = PredicateTaskCDOs[OutResult.CurrTaskIndex];
	if (!ensureAlwaysMsgf(IsValid(PredicateTask), TEXT("Invalid Predicate Task.")))
	{
		return false;
	}

	const bool bShouldWait = (IsValid(PredicateTask) && PredicateTask->WaitUntilDone(OutResult));
	return bShouldWait;
}
