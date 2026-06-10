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
#include "AutomatedTestSkillActor.h"
#include "AVVMFileHelper.h"
#include "AVVMGameplaySettings.h"
#include "NativeGameplayTags.h"
#include "Misc/AutomationTest.h"
#include "Engine/AssetManager.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

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
 *	FTestSkillSamplePluginBase define the base impl of an Automation Test, storing heap allocated data relevant for the running
 *	automation test.
 */
class FTestSkillSamplePluginBase : public FAutomationTestBase
{
public:
	FTestSkillSamplePluginBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

	virtual ~FTestSkillSamplePluginBase() override
	{
	}

	void UnitTest_PreResourceLoaded()
	{
	}

	void Setup()
	{
		TestWorld = MakeShared<FTestWorldWrapper>();
		TestWorld->CreateTestWorld(EWorldType::Game);
		TestWorld->BeginPlayInTestWorld();

		UWorld* ProxyWorld = TestWorld->GetTestWorld();
		TestNotNull("UWorld.", ProxyWorld);

		TestActor = TStrongObjectPtr(ProxyWorld->SpawnActor<AAutomatedTestSkillActor>());
		TestNotNull("AAutomatedTestInventoryActor.", TestActor.Get());

		// Restore initialization state
		TestActor->PreInitializeComponents();
		TestActor->InitializeComponents();
		TestActor->PostInitializeComponents();
		TestActor->DispatchBeginPlay();

		bIsResourceManagerAsyncCommandComplete = MakeShared<bool>(false);
		TestActor->SetTestFlag(bIsResourceManagerAsyncCommandComplete);
	}

	void LatentExecuteResourceLoading() const
	{
		ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([WeakTestActor = TWeakObjectPtr(TestActor.Get()), Flag = TSharedPtr<bool>(bIsResourceManagerAsyncCommandComplete)]
		{
			if (!WeakTestActor.IsValid())
			{
				(*Flag) = true;
				return true;
			}
		
			UAVVMResourceManagerComponent* ResourceManagerComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(WeakTestActor.Get());
			if (!IsValid(ResourceManagerComponent))
			{
				WeakTestActor->ForceCompletion();
				return true;
			}
		
			TArray<FDataRegistryId> ResourcesIds = IAVVMResourceProvider::Execute_GetResourceDefinitionRegistryIds(WeakTestActor.Get());
			if (ResourcesIds.IsEmpty())
			{
				WeakTestActor->ForceCompletion();
				return true;
			}

			for (const FDataRegistryId& ResourceId : ResourcesIds)
			{
				ResourceManagerComponent->RequestAsyncLoading(ResourceId, WeakTestActor->GetOnCompleteDelegate());
			}
		
			return true;
		}));
	}

	void LatentWait() const
	{
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForTrue(&bIsResourceManagerAsyncCommandComplete.Get()));
	}

	void LatentResourceManagerCompare()
	{
		// @gdemers validate our resource loading request integrity
		ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([this, WeakTestActor = TWeakObjectPtr(TestActor.Get())]
		{
			TestTrue("Resources loaded don't match the number requested!", WeakTestActor.IsValid() ? WeakTestActor->CheckContentIntegrity() : false);
			return true;
		}));
	}

	void LatentCleanup()
	{
		// @gdemers cleanup
		ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([this]
		{
			if (TestActor.IsValid())
			{
				TestActor->RouteEndPlay(EEndPlayReason::RemovedFromWorld);
			}
		
			if (TestWorld.IsValid())
			{
				TestWorld->EndPlayInTestWorld();
			}
		
			TestActor.Reset();
			TestWorld.Reset();
			return true;
		}));
	}

	TStrongObjectPtr<AAutomatedTestSkillActor> TestActor = nullptr;
	TSharedRef<bool> bIsResourceManagerAsyncCommandComplete = MakeShared<bool>(false);
	TSharedPtr<FTestWorldWrapper> TestWorld = nullptr;
};

/**
 *	Class description:
 *	
 *	SkillSampleTest is an Automated Test running validation on resource loading process, and granting of tree node effects using data provided
 *	from Data Asset and/or Backend.
 */
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(SkillSampleTest, FTestSkillSamplePluginBase, "AutomatedTest.CustomGroup.SkillSampleTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool SkillSampleTest::RunTest(const FString& Parameters)
{
#if WITH_AUTOMATION_TESTS
	UnitTest_PreResourceLoaded();
	Setup();
	LatentExecuteResourceLoading();
	LatentWait();
	LatentResourceManagerCompare();
	LatentCleanup();
#endif
	return true;
}