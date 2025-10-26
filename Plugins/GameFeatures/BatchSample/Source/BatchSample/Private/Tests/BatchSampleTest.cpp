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
#include "BatchSampleTest.h"

#include "AutomatedTestBatchableActor.h"
#include "AVVMWorldSetting.h"
#include "BatchingRule.h"
#include "BatchingSubsystem.h"
#include "EngineUtils.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(AUTOMATED_TEST_TAG_WORLD_RULE_BATCHING, "WorldRule.Batching");

void ABatchSampleTest::PrepareTest()
{
	Super::PrepareTest();

	const UWorld* World = GetWorld();
	AssertIsValid(this, TEXT("Expect UWorld to be valid."), World);

	auto* TestSetting = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	AssertIsValid(this, TEXT("Expect AAVVMWorldSetting to be valid."), TestSetting);

	WorldSetting = TestSetting;

	const bool bShouldCreateRule = TestSetting->ShouldCreateRule(AUTOMATED_TEST_TAG_WORLD_RULE_BATCHING);
	AssertEqual_Bool(bShouldCreateRule, true, TEXT("Expect \"AUTOMATED_TEST_TAG_WORLD_RULE_BATCHING\" to be referenced in World Settings."));

	auto* TestSubsystem = UBatchingSubsystem::Get(World);
	AssertIsValid(this, TEXT("Expect UBatchingSubsystem to be valid. Check UBatchingSubsystem::ShouldCreateSubsystem function definition."), TestSubsystem);

	BatchSubsystem = TestSubsystem;
}

bool ABatchSampleTest::IsReady_Implementation()
{
	return WorldSetting.IsValid() && BatchSubsystem.IsValid();
}

void ABatchSampleTest::StartTest()
{
	Super::StartTest();
}

void ABatchSampleTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	Super::FinishTest(TestResult, Message);
	WorldSetting.Reset();
	BatchSubsystem.Reset();
	BatchRule.Reset();
}

void ABatchSampleTest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDoesTestRun)
	{
		return;
	}
	
	RequestRuleUntilAvailable();
	if (HasRule())
	{
		RunTest_Internal();
	}
}

bool ABatchSampleTest::HasRule() const
{
	return BatchRule.IsValid();
}

void ABatchSampleTest::RequestRuleUntilAvailable()
{
	const AAVVMWorldSetting* TestSettings = WorldSetting.Get();
	if (!IsValid(TestSettings))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect AAVVMWorldSetting to be valid."));
		return;
	}

	const auto* TestRule = TestSettings->GetRule<UBatchingRule>(AUTOMATED_TEST_TAG_WORLD_RULE_BATCHING);
	if (IsValid(TestRule))
	{
		BatchRule = TestRule;
	}
}

void ABatchSampleTest::RunTest_Internal()
{
	bDoesTestRun = true;
	
	UBatchingSubsystem* TestSubsystem = BatchSubsystem.Get();
	if (!IsValid(TestSubsystem))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UBatchingSubsystem to be valid."));
		return;
	}
	
	const UBatchingRule* TestRule = BatchRule.Get();
	if (!IsValid(TestRule))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UBatchingRule to be valid."));
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UWorld to be valid."));
		return;
	}

	IgnoredWorldActors.Reset();
	TestSubsystem->Clear();

	for (TActorIterator<AAutomatedTestBatchableActor> Iterator(World); Iterator; ++Iterator)
	{
		AAutomatedTestBatchableActor* TestActor = *Iterator;
		IgnoredWorldActors.Add(TestActor);
		TestSubsystem->Register(TestActor);
	}

	TArray<AAutomatedTestBatchableActor*> TestActors;
	for (int32 i = 0; i < BatchRule->GetMaxSizePerBatchDestroy(); ++i)
	{
		auto* TestActor = World->SpawnActor<AAutomatedTestBatchableActor>();
		TestActors.Add(TestActor);
		TestSubsystem->Register(TestActor);
	}

	// TODO @gdemers Finish this impl. Dont have much time today.
}
