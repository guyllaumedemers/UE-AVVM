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
#include "AutomatedTestInventoryActor.h"
#include "AVVMGameplaySettings.h"
#include "DataRegistrySubsystem.h"
#include "InventoryFileHelper.h"
#include "InventoryUtils.h"
#include "ItemObject.h"
#include "NativeGameplayTags.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Misc/AutomationTest.h"

#include "Engine/AssetManager.h"

#if WITH_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#endif

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_INVENTORYSAMPLE_AUTOMATED_TEST, "InventorySample.AutomatedTest.Tag");

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
	
	void RunUnitTests()
	{
		RWStubInventory();
		RWDataTableInventory();
	}

	void RWStubInventory()
	{
		// @gdemers test data serialization/deserialization to disk using stub data.
		const int32 StubProviderId_A = FMath::Rand32();

		TMap<FGameplayTag, int32> StubLoadout_A =
		{
				{TAG_INVENTORYSAMPLE_AUTOMATED_TEST, FMath::Rand32()}
		};

		TArray<int32> StubItems_A = {FMath::Rand32()};

		const FString Payload = UInventoryUtils::CreateInventoryProvider(StubProviderId_A,
		                                                                 StubLoadout_A,
		                                                                 StubItems_A);

		int32 OutStubProviderId_B = INDEX_NONE;
		TMap<FGameplayTag, int32> OutStubLoadout_B;
		TArray<int32> OutStubItems_B;

		UInventoryUtils::GetInventoryProvider(Payload,
		                                      OutStubProviderId_B,
		                                      OutStubLoadout_B,
		                                      OutStubItems_B);

		TestEqual("ProviderId Equality",
		          StubProviderId_A,
		          OutStubProviderId_B);

		TestTrue("Loadout Equality",
		         StubLoadout_A.OrderIndependentCompareEqual(OutStubLoadout_B));

		TestEqual("Items Equality",
		          StubItems_A,
		          OutStubItems_B);
	}

	void RWDataTableInventory()
	{
		// @gdemers test data serialization/deserialization to disk using Data Table data.
		const FStringView FileContent = UInventoryFileHelper::Static_GetSetFileContent(true/*always test from scratch*/);
		TestFalse("Write to disk with empty content.", FileContent.IsEmpty());

		const TArray<FString> OutInventoryProviders = UInventoryUtils::GetInventoryProviderPayloads(FileContent.GetData());
		TestFalse("Read from Payload on Empty set.", OutInventoryProviders.IsEmpty());

		auto* Subsystem = UDataRegistrySubsystem::Get();
		TestNotNull("DataRegistry Subsystem", Subsystem);

		const UDataRegistry* SearchResult = Subsystem->GetRegistryForType(UAVVMGameplaySettings::GetActorIdentifierRegistryType());
		TestNotNull("DataRegistry Asset", SearchResult);

		TArray<const FAVVMActorIdentifierDataTableRow*> OutActorIdentifiers;
		SearchResult->GetAllItems<FAVVMActorIdentifierDataTableRow>(TEXT(""), OutActorIdentifiers);

		const auto CheckActorIdentifier = [](const int32 ActorIdentifier, const TArray<const FAVVMActorIdentifierDataTableRow*>& Rows)
		{
			const auto* SearchResult = Rows.FindByPredicate([ActorIdentifier](const FAVVMActorIdentifierDataTableRow* Row)
			{
				return (Row != nullptr) && (Row->UniqueId == ActorIdentifier);
			});

			return (SearchResult != nullptr);
		};

		// @gdemers validate that our providers from data table exist in the ActorIdentifier Data Table.
		for (const FString& Payload : OutInventoryProviders)
		{
			int32 OutProviderId = INDEX_NONE;
			TMap<FGameplayTag, int32> OutLoadout;
			TArray<int32> OutItems;

			UInventoryUtils::GetInventoryProvider(Payload, OutProviderId, OutLoadout, OutItems);

			const bool bResult = CheckActorIdentifier(OutProviderId, OutActorIdentifiers);
			TestTrue("ActorIdentifier missing", bResult);
		}
	}

	void Setup()
	{
		// TestWorld = MakeShared<FTestWorldWrapper>();
		// TestWorld->CreateTestWorld(EWorldType::Game);
		// TestWorld->BeginPlayInTestWorld();
		//
		// UWorld* ProxyWorld = TestWorld->GetTestWorld();
		// TestNotNull("UWorld.", ProxyWorld);
		//
		// TestActor = TStrongObjectPtr(ProxyWorld->SpawnActor<AAutomatedTestInventoryActor>());
		// TestNotNull("AAVVMAutomatedTestGameplayActor.", TestActor.Get());
		//
		// // Restore initialization state
		// TestActor->PreInitializeComponents();
		// TestActor->InitializeComponents();
		// TestActor->PostInitializeComponents();
		// TestActor->DispatchBeginPlay();
		//
		// bIsResourceManagerAsyncCommandComplete = MakeShared<bool>(false);
		// TestActor->SetTestFlag(bIsResourceManagerAsyncCommandComplete);
	}

	void LatentExecuteResourceLoading() const
	{
	}

	void LatentWait() const
	{
		// ADD_LATENT_AUTOMATION_COMMAND(FWaitForTrue(&bIsResourceManagerAsyncCommandComplete.Get()));
	}

	void LatentResourceManagerCompare()
	{
	}

	void LatentWaitInventoryStreamingHandleComplete()
	{
	}

	void LatentInventoryCompare()
	{
	}

	void LatentCleanup()
	{
		// // @gdemers cleanup
		// ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([this]
		// {
		// 	if (TestActor.IsValid())
		// 	{
		// 		TestActor->RouteEndPlay(EEndPlayReason::RemovedFromWorld);
		// 	}
		//
		// 	if (TestWorld.IsValid())
		// 	{
		// 		TestWorld->EndPlayInTestWorld();
		// 	}
		//
		// 	TestActor.Reset();
		// 	TestWorld.Reset();
		// 	return true;
		// }));
	}

	TStrongObjectPtr<AAutomatedTestInventoryActor> TestActor = nullptr;
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
	// test b : validating the loading process of a collection set provided by the FromDataAsset api call. With tag blocking loading process, and without.
	
	// test c : validating the loading process of a collection set provided by the FromMicroService api call. With tag blocking loading process, and without.
	
	// test d : test the private id actions being applied. Adding storage/removing storage, parsing encoding, etc... test backend, and disk
	
	// test e : test stacking item into the inventory. test bounds check
	
	// test f : testing the loadout object
	
	// Note : Test related to gameplay behaviour should be run on the functional test

#if WITH_AUTOMATION_TESTS
	// Setup();
	RunUnitTests();
	// LatentExecuteResourceLoading();
	// LatentWait();
	// LatentResourceManagerCompare();
	// LatentWaitInventoryStreamingHandleComplete();
	// LatentInventoryCompare();
	// LatentCleanup();
#endif
	return true;
}