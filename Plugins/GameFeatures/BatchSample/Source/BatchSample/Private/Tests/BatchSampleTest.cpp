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

	UWorld* World = GetWorld();
	AssertIsValid(World, TEXT("Expect UWorld to be valid."), this);

	auto* TestSetting = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	AssertIsValid(TestSetting, TEXT("Expect AAVVMWorldSetting to be valid."), this);

	WorldSetting = TestSetting;

	const bool bShouldCreateRule = TestSetting->ShouldCreatePluginRule(AUTOMATED_TEST_TAG_WORLD_RULE_BATCHING);
	AssertEqual_Bool(bShouldCreateRule, true, TEXT("Expect \"AUTOMATED_TEST_TAG_WORLD_RULE_BATCHING\" to be referenced in World Settings."));

	auto* TestSubsystem = UBatchingSubsystem::Get(World);
	AssertIsValid(TestSubsystem, TEXT("Expect UBatchingSubsystem to be valid. Check UBatchingSubsystem::ShouldCreateSubsystem function definition."), this);

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
		EvaluateTestPredicate();
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
		if (!IsValid(TestActor))
		{
			continue;
		}

		TestSubsystem->Register(TestActor);
		IgnoredWorldActors.Add(TestActor);
	}

	TArray<AAutomatedTestBatchableActor*> TestActors;
	for (int32 i = 0; i < FMath::CeilToInt(BatchRule->GetMaxSizePerBatchDestroy() * 1.5f)/*allow a 2nd batch that's undersized*/; ++i)
	{
		auto* TestActor = World->SpawnActor<AAutomatedTestBatchableActor>(TestActorClass);
		if (!IsValid(TestActor))
		{
			continue;
		}

		FVector Origin, BoxExtend;
		TestActor->GetActorBounds(false, Origin, BoxExtend);

		const FVector Displacement = (FMath::RandRange(0.f, 1500.f) * FVector::ForwardVector) + (FMath::RandRange(0.f, 1500.f) * FVector::RightVector) + (FVector::UpVector * BoxExtend.Z);
		TestActor->SetActorLocation(Origin + Displacement);

		TestSubsystem->Register(TestActor);
		TestActors.Add(TestActor);
	}
}

void ABatchSampleTest::EvaluateTestPredicate()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UWorld to be valid."));
		return;
	}

	UBatchingSubsystem* TestSubsystem = BatchSubsystem.Get();
	if (!IsValid(TestSubsystem))
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Expect UBatchingSubsystem to be valid."));
		return;
	}

#if WITH_AUTOMATION_TESTS
	const bool bIsEmpty = TestSubsystem->IsEmpty();
	if (!bIsEmpty)
	{
		return;
	}

	TArray<AAutomatedTestBatchableActor*> WorldActors;
	for (TActorIterator<AAutomatedTestBatchableActor> Iterator(World); Iterator; ++Iterator)
	{
		AAutomatedTestBatchableActor* TestActor = *Iterator;
		if (IsValid(TestActor))
		{
			WorldActors.Add(TestActor);
		}
	}

	const bool bAreEqual = (WorldActors.Num() == IgnoredWorldActors.Num());
	const EFunctionalTestResult TestResult = bAreEqual ? EFunctionalTestResult::Succeeded : EFunctionalTestResult::Failed;
	const FString Msg = bAreEqual ? TEXT("Batch Destroy process behaved has expected.") : TEXT("Expected World actors to be ignored, and not destroyed. Or Actors from batch weren't fully destroyed.");
	FinishTest(TestResult, Msg);
#else
	FinishTest(EFunctionalTestResult::Succeeded, TEXT("Batch Destroy process behaved has expected."));
#endif
}
