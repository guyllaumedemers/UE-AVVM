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
#pragma once

#include "CoreMinimal.h"

#include "FunctionalTest.h"

#include "BatchSampleTest.generated.h"

class AAutomatedTestBatchableActor;
class AAVVMWorldSetting;
class UBatchingRule;
class UBatchingSubsystem;

/**
 *	Class description:
 *
 *	ABatchSampleTest is a functional test run in a level blueprint that expect ULevel::WorldSetting to be
 *	configured in order to allow subsystem initialization of the BatchSample plugin and execute Tests.
 */
UCLASS()
class BATCHSAMPLE_API ABatchSampleTest : public AFunctionalTest
{
	GENERATED_BODY()

protected:
	virtual void PrepareTest() override;
	virtual bool IsReady_Implementation() override;
	virtual void StartTest() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;
	virtual void Tick(float DeltaSeconds) override;

	bool HasRule() const;
	void RequestRuleUntilAvailable();
	void RunTest_Internal();

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UBatchingSubsystem> BatchSubsystem = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UBatchingRule> BatchRule = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AAVVMWorldSetting> WorldSetting = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bDoesTestRun = false;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AAutomatedTestBatchableActor>> IgnoredWorldActors;
};
