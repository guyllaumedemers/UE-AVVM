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
#include "AVVMGameMode.h"

#include "AVVMWorldSetting.h"
#include "GameFramework/GameState.h"

AAVVMGameMode::AAVVMGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickBatching = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	SetReplicateMovement(false);
	bReplicates = false;
}

void AAVVMGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GameStateSetEvent.AddUObject(this, &AAVVMGameMode::OnGameStateSet);
		OnGameStateSet(GameState);
	}
}

void AAVVMGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GameStateSetEvent.RemoveAll(this);
	}
}

void AAVVMGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasMatchEnded())
	{
		Terminate();
	}
}

bool AAVVMGameMode::IsMatchInProgress() const
{
	bool bIsInProgress = Super::IsMatchInProgress();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchProgressTag);
		bIsInProgress &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bIsInProgress);
	}

	return bIsInProgress;
}

bool AAVVMGameMode::ReadyToStartMatch_Implementation()
{
	bool bHasStarted = Super::ReadyToStartMatch_Implementation();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchStartTag);
		bHasStarted &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bHasStarted);
	}

	return bHasStarted;
}

bool AAVVMGameMode::ReadyToEndMatch_Implementation()
{
	bool bHasEnded = Super::ReadyToEndMatch_Implementation();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchEndTag);
		bHasEnded |= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bHasEnded);
	}

	return bHasEnded;
}

bool AAVVMGameMode::HasMatchEnded() const
{
	bool bHasEnded = Super::HasMatchEnded();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchPostEndTag);
		bHasEnded &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : bHasEnded);
	}

	return bHasEnded;
}

void AAVVMGameMode::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (IsValid(NewGameState))
	{
		WorldSetting = Cast<AAVVMWorldSetting>(NewGameState->GetWorldSettings());
	}
}

void AAVVMGameMode::Terminate()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		TerminateDedicatedServer();
	}
	else
	{
		TerminateListenServer();
	}
}

void AAVVMGameMode::TerminateDedicatedServer()
{
	if (bAllowServerProcessExit)
	{
		// @gdemers By Killing the server process, the Engine::Tick will trigger a ClientTravel for
		// all players back to the Default map defined in your .ini file.
		FPlatformMisc::RequestExit(true);
	}
	else
	{
		// TODO @gdemers handle PC seamless travel back to lobby.
		RecycleProcess();
	}
}

void AAVVMGameMode::TerminateListenServer()
{
	// TODO @gdemers handle PC seamless travel back to lobby.
	RecycleProcess();
}

void AAVVMGameMode::RecycleProcess()
{
}
