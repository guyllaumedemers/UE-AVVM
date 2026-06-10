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
#include "ActorInventoryComponent.h"
#include "AutomatedTestInventoryActor.h"
#include "AVVMFileHelper.h"
#include "AVVMGameplaySettings.h"
#include "DataRegistrySubsystem.h"
#include "InventoryUtils.h"
#include "ItemObject.h"
#include "NativeGameplayTags.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Engine/AssetManager.h"
#include "Misc/AutomationTest.h"

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

	void UnitTest_PreResourceLoaded()
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
		// @gdemers lambda to conditionally generate our default provider content
		// for serialization to disk.
		static const auto GenerateDefaultContent = []()
		{
			return UInventoryUtils::CreateDefaultInventoryProviders();
		};
		
		// @gdemers test data serialization/deserialization to disk using Data Table data.
		const FStringView FileContent = UAVVMFileHelper::Static_GetSetFileContent(GenerateDefaultContent, true/*always test from scratch*/);
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

	void RWItemPrivateId()
	{
		auto* Subsystem = UDataRegistrySubsystem::Get();
		TestNotNull("DataRegistry Subsystem", Subsystem);

		const UDataRegistry* SearchResult = Subsystem->GetRegistryForType(UAVVMGameplaySettings::GetActorIdentifierRegistryType());
		TestNotNull("DataRegistry Asset", SearchResult);

		TArray<const FAVVMActorIdentifierDataTableRow*> OutActorIdentifiers;
		SearchResult->GetAllItems<FAVVMActorIdentifierDataTableRow>(TEXT(""), OutActorIdentifiers);

		const bool bResult_RunTest_ItemUniqueId = TestActor->RunTest_ItemUniqueId(OutActorIdentifiers);
		TestTrue("RunTest_ItemUniqueId", bResult_RunTest_ItemUniqueId);

		const bool bResult_RunTest_ItemStorageReference = TestActor->RunTest_ItemStorageReference();
		TestTrue("RunTest_ItemStorageReference", bResult_RunTest_ItemStorageReference);

		const bool bResult_RunTest_ItemStacking = TestActor->RunTest_ItemStacking();
		TestTrue("RunTest_ItemStacking", bResult_RunTest_ItemStacking);
	}

	void RWInventory()
	{
		const bool bResult_RunTest_InventoryBounds = TestActor->RunTest_InventoryBounds();
		TestTrue("RunTest_InventoryBounds", bResult_RunTest_InventoryBounds);
	}

	void RWLoadout()
	{
	}

	void Setup()
	{
		TestWorld = MakeShared<FTestWorldWrapper>();
		TestWorld->CreateTestWorld(EWorldType::Game);
		TestWorld->BeginPlayInTestWorld();

		UWorld* ProxyWorld = TestWorld->GetTestWorld();
		TestNotNull("UWorld.", ProxyWorld);

		TestActor = TStrongObjectPtr(ProxyWorld->SpawnActor<AAutomatedTestInventoryActor>());
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

	void LatentWaitInventoryStreamingHandleComplete()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([this, WeakTestActor = TWeakObjectPtr(TestActor.Get())]
		{
			return WeakTestActor.IsValid() && WeakTestActor->HasInventoryFinishedAllStreaming();
		}));
	}

	void LatentInventoryCompare()
	{
		// @gdemers validate our resource loading request integrity
		ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([this, WeakTestActor = TWeakObjectPtr(TestActor.Get())]
		{
			TestTrue("Resources loaded don't match the number requested!", WeakTestActor.IsValid() ? WeakTestActor->CheckInventoryIntegrity() : false);
			return true;
		}));
	}

	void LatentUnitTests()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FExecuteFunction([this]
		{
			RWItemPrivateId();
			RWInventory();
			RWLoadout();
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
	// test f : testing the loadout object
	
	// Note : Test related to gameplay behaviour should be run on the functional test

#if WITH_AUTOMATION_TESTS
	UnitTest_PreResourceLoaded();
	Setup();
	LatentExecuteResourceLoading();
	LatentWait();
	LatentResourceManagerCompare();
	LatentWaitInventoryStreamingHandleComplete();
	LatentInventoryCompare();
	LatentUnitTests();
	LatentCleanup();
#endif
	return true;
}