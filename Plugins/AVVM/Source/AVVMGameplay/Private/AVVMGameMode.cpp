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

bool AAVVMGameMode::HasMatchStarted() const
{
	bool bHasStarted = Super::HasMatchStarted();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchStartTag);
		bHasStarted &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : false);
	}

	return bHasStarted;
}

bool AAVVMGameMode::HasMatchEnded() const
{
	bool bHasEnded = Super::HasMatchEnded();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchEndTag);
		bHasEnded |= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : false);
	}

	return bHasEnded;
}

bool AAVVMGameMode::IsMatchInProgress() const
{
	bool bIsInProgress = Super::IsMatchInProgress();
	if (WorldSetting.IsValid())
	{
		const UAVVMWorldRule* MatchProgressionRule = WorldSetting->GetRule(RuleTagAggregator.MatchProgressTag);
		bIsInProgress &= (IsValid(MatchProgressionRule) ? MatchProgressionRule->Predicate() : false);
	}

	return bIsInProgress;
}

void AAVVMGameMode::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (IsValid(NewGameState))
	{
		WorldSetting = Cast<AAVVMWorldSetting>(NewGameState->GetWorldSettings());
	}
}
