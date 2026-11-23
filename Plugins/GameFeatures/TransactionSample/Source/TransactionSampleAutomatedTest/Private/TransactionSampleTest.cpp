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

#include "AutomatedTestGameState.h"
#include "AutomatedTestTransactionActor.h"
#include "FastTransactionHistoryComponent.h"
#include "TransactionHistoryComponent.h"
#include "TransactionTestTags.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Tests/AutomationCommon.h"
#include "Tests/NetTestHelpers.h"

/**
 *	Class description:
 *
 *	TransactionSampleTest is an Automated Test running validation on system feature.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransactionSampleTest, "AutomatedTest.CustomGroup.TransactionSampleTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool TransactionSampleTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper WorldWrapper;
	WorldWrapper.CreateTestWorld(EWorldType::Game);
	WorldWrapper.BeginPlayInTestWorld();
	
	UWorld* World = WorldWrapper.GetTestWorld();
	UTEST_NOT_NULL("UWorld.", World)

	AGameStateBase* GameState = World->GetGameState();
	UTEST_NOT_NULL("AGameStateBase.", GameState)

	auto* TestComponent = GameState->GetComponentByClass<UTransactionHistoryComponent>();
	if (!IsValid(TestComponent))
	{
		auto* NewComponent = GameState->AddComponentByClass(UTransactionHistoryComponent::StaticClass(), false, FTransform::Identity, false);
		TestComponent = Cast<UTransactionHistoryComponent>(NewComponent);
	}

	UTEST_NOT_NULL("UTransactionHistoryComponent.", TestComponent)

	const auto* TestActor = World->SpawnActor<AAutomatedTestTransactionActor>();
	UTEST_NOT_NULL("AAutomatedTestTransactionActor.", TestActor)

	const FString TestActorUniqueId = UTransactionObjectUtils::GetUniqueId(TestActor);
	UTEST_FALSE("Checking Empty FString.", TestActorUniqueId.IsEmpty())
	UTEST_NOT_EQUAL("Checking UniqueId property Equality.", TestActorUniqueId, FString::FromInt(INDEX_NONE))

	// @gdemers create dummy payload.
	const auto InputPayload = FAVVMJsonPayload::To<int32>(1);
	const FAVVMJsonPayload* PolymorphicPayload = InputPayload.GetPtr<FAVVMJsonPayload>();
	UTEST_NOT_NULL("FTransactionPayload.", PolymorphicPayload)

	const int32 OutOtherValue = FAVVMJsonPayload::From<int32>(PolymorphicPayload);

	// @gdemers property comparison.
	UTEST_EQUAL("Checking Payload property Equality.", 1, OutOtherValue)

	// @gdemers testing registration system.
	FTransactionContextArgs Args_A;
	Args_A.Instigator = nullptr;
	Args_A.Target = TestActor;
	Args_A.TransactionType = ETransactionType::Kill;
	Args_A.Payload = PolymorphicPayload->JsonPayload;
	UTransactionHistoryComponent::Static_CreateAndRecordTransaction(World, Args_A);

	FTransactionContextArgs Args_B;
	Args_B.Instigator = nullptr;
	Args_B.Target = TestActor;
	Args_B.TransactionType = ETransactionType::Killstreak;
	Args_B.Payload = PolymorphicPayload->JsonPayload;
	UTransactionHistoryComponent::Static_CreateAndRecordTransaction(World, Args_B);

	// @gdemers increment existing entry.
	FTransactionContextArgs Args_C;
	Args_C.Instigator = nullptr;
	Args_C.Target = TestActor;
	Args_C.TransactionType = ETransactionType::Kill;
	Args_C.Payload = PolymorphicPayload->JsonPayload;
	UTransactionHistoryComponent::Static_CreateAndRecordTransaction(World, Args_C);

	TArray<const FTransactionObject*> OutResult_A = UTransactionHistoryComponent::Static_GetAllTransactionsOfType(World, TestActorUniqueId, ETransactionType::Kill);
	UTEST_EQUAL("Post-Addition, ETransactionType::Kill Count.", OutResult_A.Num(), 2)

	int32 OutResult = 0;
	UTransactionHistoryComponent::Static_GetAggregatedValues<int32>(World, TestActorUniqueId, ETransactionType::Kill, OutResult);
	UTEST_EQUAL("Aggregate values, ETransactionType::Kill Total Value.", OutResult, 2)

	TArray<const FTransactionObject*> OutResult_B = UTransactionHistoryComponent::Static_GetAllTransactions(World, TestActorUniqueId);
	UTEST_EQUAL("Post-Addition, All Transaction Count.", OutResult_B.Num(), 3)

	UTransactionHistoryComponent::Static_RemoveAllTransactionOfType(World, TestActor, ETransactionType::Killstreak);

	TArray<const FTransactionObject*> OutResult_C = UTransactionHistoryComponent::Static_GetAllTransactionsOfType(World, TestActorUniqueId, ETransactionType::Killstreak);
	UTEST_EQUAL("Post-Removal, ETransactionType::Killstreak Count.", OutResult_C.Num(), 0)

	UTransactionHistoryComponent::Static_RemoveAllTransactions(World, TestActor);

	TArray<const FTransactionObject*> OutResult_D = UTransactionHistoryComponent::Static_GetAllTransactions(World, TestActorUniqueId);
	UTEST_EQUAL("Post-Removal, All Transaction Count.", OutResult_D.Num(), 0)

	// @gdemers cleanup.
	WorldWrapper.EndPlayInTestWorld();
	return true;
}

/**
 *	Class description:
 *
 *	FastTransactionObjectTest is an Automated Test running validation on system feature.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FastTransactionObjectTest, "AutomatedTest.CustomGroup.FastTransactionObjectTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FastTransactionObjectTest::RunTest(const FString& Parameters)
{
	UE::Net::FTestWorlds TestWorlds(TEXT("/Engine/Maps/Entry"), TEXT("/Script/TransactionSampleAutomatedTest.TransactionNetTestGameMode"), EReplicationSystem::Iris);

	UWorld* ServerWorld = TestWorlds.Server.GetWorld();
	UTEST_NOT_NULL(TEXT("Server expect a valid UWorld"), ServerWorld);

	// @gdemers create client connection. this will ensure the server has an associated PC representing the client.
	const bool bHasClientConnection = TestWorlds.CreateAndConnectClient();
	UTEST_TRUE(TEXT("Client is expected to be Valid"), bHasClientConnection);

	const int32 ClientId{TestWorlds.FindClientIdFromWorld(TestWorlds.Clients[0].GetWorld())};
	UTEST_NOT_EQUAL(TEXT("Client is expected have Valid Id"), ClientId, static_cast<int32>(INDEX_NONE));

	const bool bDoesServerHasPC = TestWorlds.TickAllUntil([&]() { return IsValid(ServerWorld->GetFirstPlayerController()); },
	                                                      TestWorlds.GetTickDeltaSeconds(),
	                                                      120);

	UTEST_TRUE(TEXT("Server expect a valid APlayerController"), bDoesServerHasPC);

	// @gdemers fetch the client PC. theres only one in this case.
	auto* ServerPC = ServerWorld->GetFirstPlayerController();
	UTEST_TRUE(TEXT("Server expect to replicate APlayerController"), TestWorlds.IsServerObjectReplicated(ServerPC));

	const bool bDoesServerHasPlayerState = TestWorlds.TickAllUntil([&]() { return IsValid(ServerPC->GetPlayerState<APlayerState>()); },
	                                                               TestWorlds.GetTickDeltaSeconds(),
	                                                               120);

	UTEST_TRUE(TEXT("Server expect a valid APlayerState"), bDoesServerHasPlayerState);

	auto* ServerPlayerState = ServerPC->GetPlayerState<APlayerState>();
	UTEST_TRUE(TEXT("Server expect to replicate APlayerState"), TestWorlds.IsServerObjectReplicated(ServerPlayerState));

	const bool bDoesClientOwnPlayerState = TestWorlds.TickAllUntil([&]() { return TestWorlds.DoesReplicatedObjectExistOnClient(ServerPlayerState, ClientId); },
	                                                               TestWorlds.GetTickDeltaSeconds(),
	                                                               120);

	UTEST_TRUE(TEXT("Replicated actor should exist on client"), bDoesClientOwnPlayerState);

	auto* ClientPlayerState = TestWorlds.FindReplicatedObjectOnClient<APlayerState>(ServerPlayerState, ClientId);
	UTEST_NOT_NULL(TEXT("Client replicated actor instance should be found"), ClientPlayerState);

	const bool bIsPlayerUniqueNetIdReplicated = TestWorlds.TickAllUntil([&]() { return ClientPlayerState->GetUniqueId().IsValid() && (ClientPlayerState->GetUniqueId() == ServerPlayerState->GetUniqueId()); },
	                                                                    TestWorlds.GetTickDeltaSeconds(),
	                                                                    120);

	UTEST_TRUE(TEXT("Replicated actor should own a valid PlayerUniqueNetId on client"), bIsPlayerUniqueNetIdReplicated);

	auto* ServerGameState = Cast<ATransactionNetTestReplicatedGameState>(ServerWorld->GetGameState());
	UTEST_NOT_NULL(TEXT("Expect a valid Server GameState"), ServerGameState);

	const auto* ClientWorld = ClientPlayerState->GetWorld();
	auto* ClientGameState = Cast<ATransactionNetTestReplicatedGameState>(IsValid(ClientWorld) ? ClientWorld->GetGameState() : nullptr);
	UTEST_NOT_NULL(TEXT("Expect a valid Client GameState"), ClientGameState);

	UFastTransactionHistoryComponent::Static_ModifyTransaction(ServerWorld, ServerPlayerState->GetUniqueId(), TAG_TRANSACTIONSAMPLE_TEST_STAT_PLAYER, 2, true);
	const bool bUpdatedOnClient = TestWorlds.TickAllUntil([&]()
	                                                      {
		                                                      const auto* Value = UFastTransactionHistoryComponent::Static_GetFastLookupTag(ClientPlayerState, ClientPlayerState->GetUniqueId(), TAG_TRANSACTIONSAMPLE_TEST_STAT_PLAYER);
		                                                      return (Value != nullptr) ? ((Value->ReplicatedStackCount == Value->PredictedStackCount) && (Value->PredictedStackCount != INDEX_NONE)) : false;
	                                                      },
	                                                      TestWorlds.GetTickDeltaSeconds(),
	                                                      240);

	UTEST_TRUE(TEXT("Updated GameplayTagStack property should replicate to client"), bUpdatedOnClient);

	const auto* TagStack = UFastTransactionHistoryComponent::Static_GetFastLookupTag(ClientPlayerState, ClientPlayerState->GetUniqueId(), TAG_TRANSACTIONSAMPLE_TEST_STAT_PLAYER);
	UTEST_NOT_NULL(TEXT("Client GameplayTagStack should retrieve valid tag entry using fast lookup"), TagStack);
	UTEST_EQUAL(TEXT("Client GameplayTagStack Replicated Value should equal 2"), TagStack->ReplicatedStackCount, 2);
	UTEST_EQUAL(TEXT("Client GameplayTagStack Predicted Value should equal the Replicated Value"), TagStack->PredictedStackCount, TagStack->ReplicatedStackCount);
	return true;
}
