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
#include "Misc/AutomationTest.h"

#include "AVVMAutomatedTestGameplayActor.h"
#include "AVVMAutomatedTestResourceComponent.h"

#if WITH_EDITOR && WITH_AUTOMATION_TESTS
#include "Tests/AutomationEditorCommon.h"
#endif

class AAVVMAutomatedTestGameplayActor;

/**
 * 
 */
struct FAVVMResourceManagerTestHelper
{
	~FAVVMResourceManagerTestHelper() = default;
	
	bool Update()
	{
		// TODO @gdemers define latent task. require tracking progress.
		return true;
	}
	
	TWeakObjectPtr<const AAVVMAutomatedTestGameplayActor> TestActor = nullptr;
};

/**
 *	Class description:
 *
 *	FAVVMRunResourceManagerTestCommand is an Automated Test latent action to validate async process in an Automated Test scenario.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FAVVMRunResourceManagerTestCommand, TSharedPtr<FAVVMResourceManagerTestHelper>, TestHelper);
bool FAVVMRunResourceManagerTestCommand::Update()
{
	return TestHelper->Update();
}

/**
 *	Class description:
 *
 *	AVVMResourceManagerComponentTest is an Automated Test running validation on resource loading process.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AVVMResourceManagerComponentTest, "AutomatedTest.CustomGroup.AVVMResourceManagerComponentTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AVVMResourceManagerComponentTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR && WITH_AUTOMATION_TESTS
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	UTEST_NOT_NULL("UWorld.", World)

	const auto* TestActor = World->SpawnActor<AAVVMAutomatedTestGameplayActor>(AAVVMAutomatedTestGameplayActor::StaticClass());
	UTEST_NOT_NULL("AAVVMAutomatedTestGameplayActor.", TestActor)

	const TSharedPtr<FAVVMResourceManagerTestHelper> TestHelper = MakeShared<FAVVMResourceManagerTestHelper>(TestActor);
	UTEST_VALID("TSharedPtr<FAVVMResourceManagerTestHelper>", TestHelper)

	ADD_LATENT_AUTOMATION_COMMAND(FAVVMRunResourceManagerTestCommand(TestHelper));

	World->DestroyWorld(true);
#endif
	return true;
}

/**
 *	Class description:
 *
 *	AVVMAbilitySystemComponentTest is an Automated Test running validation on ability loading process and attribute initialization.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AVVMAbilitySystemComponentTest, "AutomatedTest.CustomGroup.AVVMAbilitySystemComponentTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AVVMAbilitySystemComponentTest::RunTest(const FString& Parameters)
{
	// Make the test pass by returning true, or fail by returning false.
	return true;
}
