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

#include "AVVMAutomatedTestPlayerState.h"
#include "AVVMGameMode.h"
#include "AVVMGameplayTagStackObject.h"
#include "AVVMTestTags.h"
#include "Net/UnrealNetwork.h"
#include "Tests/AutomationCommon.h"
#include "Tests/NetTestHelpers.h"

/**
 *	Class description:
 *
 *	AVVMGameplayTagStackObjectTest is an Automated Test running validation on system feature.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AVVMGameplayTagStackObjectTest, "AutomatedTest.CustomGroup.AVVMGameplayTagStackObjectTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AVVMGameplayTagStackObjectTest::RunTest(const FString& Parameters)
{
	UE::Net::FTestWorlds TestWorlds(TEXT("/Engine/Maps/Entry"), TEXT("/Script/AVVMAutomatedTest.NetTestGameMode"), EReplicationSystem::Iris);

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

	const bool bDoesServerHasPlayerState = TestWorlds.TickAllUntil([&]() { return IsValid(ServerPC->GetPlayerState<ANetTestReplicatedPlayerState>()); },
	                                                               TestWorlds.GetTickDeltaSeconds(),
	                                                               120);

	UTEST_TRUE(TEXT("Server expect a valid APlayerState"), bDoesServerHasPlayerState);

	auto* ServerPlayerState = ServerPC->GetPlayerState<ANetTestReplicatedPlayerState>();
	UTEST_TRUE(TEXT("Server expect to replicate APlayerState"), TestWorlds.IsServerObjectReplicated(ServerPlayerState));

	const bool bDoesClientOwnPlayerState = TestWorlds.TickAllUntil([&]() { return TestWorlds.DoesReplicatedObjectExistOnClient(ServerPlayerState, ClientId); },
	                                                               TestWorlds.GetTickDeltaSeconds(),
	                                                               120);

	UTEST_TRUE(TEXT("Replicated actor should exist on client"), bDoesClientOwnPlayerState);

	auto* ClientPlayerState = TestWorlds.FindReplicatedObjectOnClient<ANetTestReplicatedPlayerState>(ServerPlayerState, ClientId);
	UTEST_NOT_NULL(TEXT("Client replicated actor instance should be found"), ClientPlayerState);

	const bool bIsPlayerUniqueNetIdReplicated = TestWorlds.TickAllUntil([&]() { return ClientPlayerState->GetUniqueId().IsValid() && (ClientPlayerState->GetUniqueId() == ServerPlayerState->GetUniqueId()); },
	                                                                    TestWorlds.GetTickDeltaSeconds(),
	                                                                    120);

	UTEST_TRUE(TEXT("Replicated actor should own a valid PlayerUniqueNetId on client"), bIsPlayerUniqueNetIdReplicated);
	UTEST_EQUAL(TEXT("Initial replicated GameplayTagStack should be empty"), ClientPlayerState->ReplicatedTags, FAVVMGameplayTagStackObjectFastArray{});

	ServerPlayerState->ReplicatedTags.Modify(TAG_AVVMGAMEPLAY_TEST_STAT_PLAYER, 2, true);
	const bool bUpdatedOnClient = TestWorlds.TickAllUntil([&]()
	                                                      {
		                                                      const auto* Value = ClientPlayerState->ReplicatedTags.GetFastLookupTag(TAG_AVVMGAMEPLAY_TEST_STAT_PLAYER);
		                                                      return (Value != nullptr) ? ((Value->ReplicatedStackCount == Value->PredictedStackCount) && (Value->PredictedStackCount != INDEX_NONE)) : false;
	                                                      },
	                                                      TestWorlds.GetTickDeltaSeconds(),
	                                                      240);

	UTEST_TRUE(TEXT("Updated GameplayTagStack property should replicate to client"), bUpdatedOnClient);

	const auto* TagStack = ClientPlayerState->ReplicatedTags.GetFastLookupTag(TAG_AVVMGAMEPLAY_TEST_STAT_PLAYER);
	UTEST_NOT_NULL(TEXT("Client GameplayTagStack should retrieve valid tag entry using fast lookup"), TagStack);
	UTEST_EQUAL(TEXT("Client GameplayTagStack Replicated Value should equal 2"), TagStack->ReplicatedStackCount, 2);
	UTEST_EQUAL(TEXT("Client GameplayTagStack Predicted Value should equal the Replicated Value"), TagStack->PredictedStackCount, TagStack->ReplicatedStackCount);
	return true;
}
