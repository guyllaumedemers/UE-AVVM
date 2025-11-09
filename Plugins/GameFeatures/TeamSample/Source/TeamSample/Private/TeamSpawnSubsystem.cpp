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
#include "TeamSpawnSubsystem.h"

#include "AVVMGameplayUtils.h"
#include "AVVMWorldSetting.h"
#include "NativeGameplayTags.h"
#include "TeamSample.h"
#include "TeamSettings.h"
#include "TeamSpawnRule.h"
#include "TeamStartComponent.h"
#include "Engine/StreamableManager.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_WORLD_RULE_TEAMSPAWNING, "WorldRule.TeamSpawning");

bool UTeamSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (!IsValid(World))
	{
		return false;
	}

	const auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (!IsValid(WorldSettings) || !WorldSettings->ShouldCreatePluginRule(TAG_WORLD_RULE_TEAMSPAWNING))
	{
		return false;
	}

	const bool bDoesOverrideSubsystemCreation = UTeamSettings::DoesLevelOverrideSubsystemCreation(World->PersistentLevel);
	if (bDoesOverrideSubsystemCreation)
	{
		return true;
	}

	return !World->IsNetMode(NM_Client);
}

void UTeamSpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTeamSpawnSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UTeamSpawnSubsystem::Static_UnRegisterPlayerStart(const UWorld* World,
                                                       const UTeamStartComponent* Component)
{
	auto* TeamSpawnSubsystem = UTeamSpawnSubsystem::Get(World);
	if (IsValid(TeamSpawnSubsystem))
	{
		TeamSpawnSubsystem->UnRegister(Component);
	}
}

void UTeamSpawnSubsystem::Static_RegisterPlayerStart(const UWorld* World,
                                                     const UTeamStartComponent* Component)
{
	auto* TeamSpawnSubsystem = UTeamSpawnSubsystem::Get(World);
	if (IsValid(TeamSpawnSubsystem))
	{
		TeamSpawnSubsystem->Register(Component);
	}
}

const UTeamStartComponent* UTeamSpawnSubsystem::Static_TryGetPlayerStart(const UWorld* World,
                                                                         const FWorldContextArgs& WorldContextArgs,
                                                                         const APlayerState* OldPlayerState)
{
	auto* TeamSpawnSubsystem = UTeamSpawnSubsystem::Get(World);
	return IsValid(TeamSpawnSubsystem) ? TeamSpawnSubsystem->TryGetPlayerStart(WorldContextArgs, OldPlayerState) : nullptr;
}

UTeamSpawnSubsystem* UTeamSpawnSubsystem::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UTeamSpawnSubsystem>(World);
}

void UTeamSpawnSubsystem::UnRegister(const UTeamStartComponent* Component)
{
	PlayerStarts.Remove(Component);

	if (!IsValid(Component))
	{
		return;
	}

	const auto* Outer = Component->GetTypedOuter<AActor>();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogTeamSample,
	       Log,
	       TEXT("Executed from \"%s\". Actor \"%s\" unregistered."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())
}

void UTeamSpawnSubsystem::Register(const UTeamStartComponent* Component)
{
	PlayerStarts.Add(Component);

	if (!IsValid(Component))
	{
		return;
	}

	const auto* Outer = Component->GetTypedOuter<AActor>();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogTeamSample,
	       Log,
	       TEXT("Executed from \"%s\". Actor \"%s\" registered."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())
}

const UTeamStartComponent* UTeamSpawnSubsystem::TryGetPlayerStart(const FWorldContextArgs& WorldContextArgs,
                                                                  const APlayerState* OldPlayerState) const
{
	const UTeamStartComponent* Result = nullptr;

	for (const TWeakObjectPtr<const UTeamStartComponent>& PlayerStart : PlayerStarts)
	{
		bool bPredicate = false;

		const UTeamStartComponent* SpawnPoint = PlayerStart.Get();
		if (!IsValid(SpawnPoint))
		{
			continue;
		}

		for (const UTeamSpawnCondition* SpawnCondition : SpawnConditions)
		{
			if (!IsValid(SpawnCondition))
			{
				continue;
			}

			bPredicate &= SpawnCondition->Predicate(WorldContextArgs, SpawnPoint, OldPlayerState);
		}

		if (bPredicate)
		{
			Result = UTeamSpawnUtils::WeightChoice(SpawnWeightRule.Get(), Result, SpawnPoint);
		}
	}

	return Result;
}

void UTeamSpawnSubsystem::CreateTeamSpawnRule()
{
	const auto OnAsyncLoadComplete = [](const TWeakObjectPtr<UTeamSpawnSubsystem>& NewTeamSpawnSubsystem,
	                                    const TWeakObjectPtr<AAVVMWorldSetting>& NewWorldSettings)
	{
		if (!NewTeamSpawnSubsystem.IsValid() || !NewWorldSettings.IsValid())
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = NewTeamSpawnSubsystem->TeamRuleStreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		const auto* RuleClass = Cast<UClass>(Handle->GetLoadedAsset());
		if (!IsValid(RuleClass))
		{
			return;
		}

		NewTeamSpawnSubsystem->TeamSpawnRule = NewWorldSettings->GetOrCreatePluginRule<UTeamSpawnRule>(TAG_WORLD_RULE_TEAMSPAWNING, RuleClass);
		NewTeamSpawnSubsystem->InitRule();
	};

	const auto* World = GetTypedOuter<UWorld>();
	if (!IsValid(World))
	{
		return;
	}

	auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (IsValid(WorldSettings))
	{
		const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncLoadComplete, TWeakObjectPtr(this), TWeakObjectPtr(WorldSettings));
		TeamRuleStreamableHandle = WorldSettings->AsyncLoadPluginRule(UTeamSettings::GetTeamSpawnRuleClass(), Callback);
	}
}

void UTeamSpawnSubsystem::InitRule()
{
	CreateTeamSpawnWeightRule();
	CreateSpawnConditions();
}

void UTeamSpawnSubsystem::CreateTeamSpawnWeightRule()
{
	const UTeamSpawnRule* Rule = TeamSpawnRule.Get();
	if (!IsValid(Rule))
	{
		return;
	}

	const auto OnAsyncLoadComplete = [](const TWeakObjectPtr<UTeamSpawnSubsystem>& NewTeamSpawnSubsystem)
	{
		UTeamSpawnSubsystem* TeamSpawnSubsystem = NewTeamSpawnSubsystem.Get();
		if (!IsValid(TeamSpawnSubsystem))
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = NewTeamSpawnSubsystem->SpawnWeightRuleStreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		const auto* SpawnWeightRuleClass = Cast<UClass>(Handle->GetLoadedAsset());
		if (IsValid(SpawnWeightRuleClass))
		{
			const auto* SpawnWeightRule = NewObject<UTeamSpawnWeightRule>(TeamSpawnSubsystem, SpawnWeightRuleClass);
			TeamSpawnSubsystem->SpawnWeightRule = SpawnWeightRule;
		}
	};

	const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncLoadComplete, TWeakObjectPtr(this));
	SpawnWeightRuleStreamableHandle = UAssetManager::Get().LoadAssetList({Rule->GetSpawnWeightRule().ToSoftObjectPath()}, Callback);
}

void UTeamSpawnSubsystem::CreateSpawnConditions()
{
	const UTeamSpawnRule* Rule = TeamSpawnRule.Get();
	if (!IsValid(Rule))
	{
		return;
	}

	TArray<FSoftObjectPath> SpawnConditionPaths;
	for (const auto& SpawnConditionsClass : Rule->GetSpawnConditions())
	{
		SpawnConditionPaths.Add(SpawnConditionsClass.ToSoftObjectPath());
	}

	const auto OnAsyncLoadComplete = [](const TWeakObjectPtr<UTeamSpawnSubsystem>& NewTeamSpawnSubsystem)
	{
		UTeamSpawnSubsystem* TeamSpawnSubsystem = NewTeamSpawnSubsystem.Get();
		if (!IsValid(TeamSpawnSubsystem))
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = NewTeamSpawnSubsystem->SpawnConditionStreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		Handle->GetLoadedAssets(OutResources);

		for (const UObject* Resource : OutResources)
		{
			const auto* SpawnConditionClass = Cast<UClass>(Resource);
			if (!IsValid(SpawnConditionClass))
			{
				continue;
			}

			const auto* NewSpawnCondition = NewObject<UTeamSpawnCondition>(TeamSpawnSubsystem, SpawnConditionClass);
			TeamSpawnSubsystem->SpawnConditions.Add(NewSpawnCondition);
		}
	};

	const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncLoadComplete, TWeakObjectPtr(this));
	SpawnConditionStreamableHandle = UAssetManager::Get().LoadAssetList(SpawnConditionPaths, Callback);
}

const UTeamStartComponent* UTeamSpawnUtils::WeightChoice(const UTeamSpawnWeightRule* WeightRule,
                                                         const UTeamStartComponent* ChoiceA,
                                                         const UTeamStartComponent* ChoiceB)
{
	if (!IsValid(WeightRule) || !IsValid(ChoiceA) || !IsValid(ChoiceB))
	{
		return IsValid(ChoiceA) ? ChoiceA : ChoiceB;
	}

	return WeightRule->Weight(ChoiceA, ChoiceB);
}
