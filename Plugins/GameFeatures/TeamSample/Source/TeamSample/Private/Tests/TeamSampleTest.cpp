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
#include "TeamSampleTest.h"

#include "AVVMWorldSetting.h"
#include "GameStateTeamComponent.h"
#include "NativeGameplayTags.h"
#include "TeamRule.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

UE_DEFINE_GAMEPLAY_TAG(AUTOMATED_TEST_TAG_WORLD_RULE_TEAM, "WorldRule.Team");

void ATeamSampleTest::PrepareTest()
{
	Super::PrepareTest();

	UWorld* World = GetWorld();
	AssertIsValid(World, TEXT("Expect UWorld to be valid."), this);

	auto* TestSetting = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	AssertIsValid(TestSetting, TEXT("Expect AAVVMWorldSetting to be valid."), this);

	WorldSetting = TestSetting;

	const bool bShouldCreateRule = TestSetting->ShouldCreatePluginRule(AUTOMATED_TEST_TAG_WORLD_RULE_TEAM);
	AssertEqual_Bool(bShouldCreateRule, true, TEXT("Expect \"AUTOMATED_TEST_TAG_WORLD_RULE_TEAM\" to be referenced in World Settings."));
}

bool ATeamSampleTest::IsReady_Implementation()
{
	return WorldSetting.IsValid();
}

void ATeamSampleTest::StartTest()
{
	Super::StartTest();
}

void ATeamSampleTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	Super::FinishTest(TestResult, Message);
	WorldSetting.Reset();
	TeamRule.Reset();
}

void ATeamSampleTest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDoesTestRun)
	{
		EvaluateTestPredicate();
		return;
	}

	RequestRuleUntilAvailable();
	if (HasRule())
	{
		RunTest_Internal();
	}
}

bool ATeamSampleTest::HasRule() const
{
	return TeamRule.IsValid();
}

void ATeamSampleTest::RequestRuleUntilAvailable()
{
	const AAVVMWorldSetting* TestSettings = WorldSetting.Get();
	if (!IsValid(TestSettings))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect AAVVMWorldSetting to be valid."));
		return;
	}

	const auto* TestRule = TestSettings->GetRule<UTeamRule>(AUTOMATED_TEST_TAG_WORLD_RULE_TEAM);
	if (IsValid(TestRule))
	{
		TeamRule = TestRule;
	}
}

void ATeamSampleTest::RunTest_Internal()
{
	bDoesTestRun = true;

	const UTeamRule* TestRule = TeamRule.Get();
	if (!IsValid(TestRule))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UTeamRule to be valid."));
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UWorld to be valid."));
		return;
	}

	AGameStateBase* GameState = UGameplayStatics::GetGameState(World);
	if (!IsValid(GameState))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect AGameStateBase to be valid."));
		return;
	}

	auto* TestComponent = GameState->GetComponentByClass<UGameStateTeamComponent>();
	if (!IsValid(TestComponent))
	{
		auto* NewComponent = GameState->AddComponentByClass(UGameStateTeamComponent::StaticClass(), false, FTransform::Identity, false);
		TestComponent = Cast<UGameStateTeamComponent>(NewComponent);
	}

	if (!IsValid(TestComponent))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UGameStateTeamComponent to be valid."));
		return;
	}

	TestComponent->BeginPlay();

	TArray<APlayerState*> PlayerStates;
	PlayerStates.Add(World->SpawnActor<APlayerState>());
	PlayerStates.Add(World->SpawnActor<APlayerState>());
	PlayerStates.Add(World->SpawnActor<APlayerState>());

	for (APlayerState* PlayerState : PlayerStates)
	{
		GameState->AddPlayerState(PlayerState);
	}
}

void ATeamSampleTest::EvaluateTestPredicate()
{
}
