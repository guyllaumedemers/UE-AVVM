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

#include "Engine/AssetManager.h"

#if WITH_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#endif

// @gdemers from GameFeaturePluginTests.cpp
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForTrue, bool*, bVariableToWaitFor);
bool FWaitForTrue::Update()
{
	// @gdemers : careful, we want to ensure all dependent resources are loaded without being affected by unrelated loading logic. if you have some external system doing async
	// load, and waiting, this will create throttling.
	return *bVariableToWaitFor && UAssetManager::GetStreamableManager().AreAllAsyncLoadsComplete();
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FExecuteFunction, TFunction<bool()>, Function);
bool FExecuteFunction::Update()
{
	return Function();
}

/**
 *	Class description:
 *	
 *	FTestInventorySamplePluginBase define the base impl of an Automation Test, storing heap allocated data relevant for the running
 *	automation test.
 */
class FTestInventorySamplePluginBase : public FAutomationTestBase
{
public:
	FTestInventorySamplePluginBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

	virtual ~FTestInventorySamplePluginBase() override
	{
	}

	TSharedRef<bool> bIsResourceManagerAsyncCommandComplete = MakeShared<bool>(false);
	TSharedPtr<FTestWorldWrapper> TestWorld = nullptr;
};

/**
 *	Class description:
 *	
 *	InventorySampleTest is an Automated Test running validation on resource loading process, and spawning/socketing of items using data provided
 *	from Data Asset and/or Backend.
 */
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(InventorySampleTest, FTestInventorySamplePluginBase, "AutomatedTest.CustomGroup.InventorySampleTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool InventorySampleTest::RunTest(const FString& Parameters)
{
	// Make the test pass by returning true, or fail by returning false.
	return true;
}