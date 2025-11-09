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
#include "TeamStartComponent.h"

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
