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

#include "AutomatedTestTransactionActor.h"
#include "GameStateTransactionHistory.h"
#include "TransactionFactoryImplTest.h"
#include "GameFramework/GameStateBase.h"

#if WITH_EDITOR && WITH_AUTOMATION_TESTS
#include "Tests/AutomationEditorCommon.h"
#endif

/**
 *	Class description:
 *
 *	TransactionSampleTest is an Automated Test running validation on system feature.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TransactionSampleTest, "AutomatedTest.CustomGroup.TransactionSampleTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool TransactionSampleTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR && WITH_AUTOMATION_TESTS
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	UTEST_NOT_NULL("UWorld.", World)

	auto* GameState = World->SpawnActor<AGameStateBase>();
	UTEST_NOT_NULL("AGameStateBase.", GameState)

	// @gdemers required manual registration.
	World->SetGameState(GameState);

	auto* TestComponent = GameState->GetComponentByClass<UGameStateTransactionHistory>();
	if (!IsValid(TestComponent))
	{
		auto* NewComponent = GameState->AddComponentByClass(UGameStateTransactionHistory::StaticClass(), false, FTransform::Identity, false);
		TestComponent = Cast<UGameStateTransactionHistory>(NewComponent);
	}

	UTEST_NOT_NULL("UGameStateTransactionHistory.", TestComponent)

	// @gdemers required manual invocation.
	TestComponent->BeginPlay();

	const auto* TestActor = World->SpawnActor<AAutomatedTestTransactionActor>();
	UTEST_NOT_NULL("AAutomatedTestTransactionActor.", TestActor)

	const FString TestActorUniqueId = UTransaction::GetUniqueId(TestActor);
	UTEST_FALSE("Checking Empty FString.", TestActorUniqueId.IsEmpty())
	UTEST_NOT_EQUAL("Checking UniqueId property Equality.", TestActorUniqueId, FString::FromInt(INDEX_NONE))

	// @gdemers create dummy payload.
	const TInstancedStruct<FTransactionPayload> InputPayload = FTransactionPayload::Make<FTransactionPayloadTest>(1);
	const auto* PolymorphicPayload = InputPayload.GetPtr<FTransactionPayloadTest>();
	UTEST_NOT_NULL("FTransactionPayload.", PolymorphicPayload)

	// @gdemers testing utility api.
	const FString StringInput = UTransactionFactoryUtils::CreateStringPayload(InputPayload);
	const TInstancedStruct<FTransactionPayload> OutputPayload = UTransactionFactoryUtils::CreatePayloadFromString(UTransactionFactoryImplTest::StaticClass(), StringInput);
	const auto* OtherPolymorphicPayload = OutputPayload.GetPtr<FTransactionPayloadTest>();
	UTEST_NOT_NULL("FTransactionPayload.", OtherPolymorphicPayload)

	// @gdemers property comparison.
	UTEST_EQUAL("Checking Payload property Equality.", PolymorphicPayload->Value, OtherPolymorphicPayload->Value)

	// @gdemers testing registration system.
	FTransactionContextArgs Args_A;
	Args_A.Instigator = nullptr;
	Args_A.Target = TestActor;
	Args_A.TransactionType = ETransactionType::Kill;
	Args_A.Payload = StringInput;
	UGameStateTransactionHistory::Static_CreateAndRecordTransaction(World, Args_A);

	FTransactionContextArgs Args_B;
	Args_B.Instigator = nullptr;
	Args_B.Target = TestActor;
	Args_B.TransactionType = ETransactionType::Killstreak;
	Args_B.Payload = StringInput;
	UGameStateTransactionHistory::Static_CreateAndRecordTransaction(World, Args_B);

	// @gdemers increment existing entry.
	FTransactionContextArgs Args_C;
	Args_C.Instigator = nullptr;
	Args_C.Target = TestActor;
	Args_C.TransactionType = ETransactionType::Kill;
	Args_C.Payload = StringInput;
	UGameStateTransactionHistory::Static_CreateAndRecordTransaction(World, Args_C);

	TArray<const UTransaction*> OutResult_A = UGameStateTransactionHistory::Static_GetAllTransactionsOfType(World, TestActorUniqueId, ETransactionType::Kill);
	UTEST_EQUAL("Post-Addition, ETransactionType::Kill Count.", OutResult_A.Num(), 2)

	int32 OutResult = 0;
	UGameStateTransactionHistory::Static_GetAggregatedValues<FTransactionPayloadTest, int32>(World, TestActorUniqueId, ETransactionType::Kill, OutResult);
	UTEST_EQUAL("Aggregate values, ETransactionType::Kill Total Value.", OutResult, 2)

	TArray<const UTransaction*> OutResult_B = UGameStateTransactionHistory::Static_GetAllTransactions(World, TestActorUniqueId);
	UTEST_EQUAL("Post-Addition, All Transaction Count.", OutResult_B.Num(), 3)

	UGameStateTransactionHistory::Static_RemoveAllTransactionOfType(World, TestActor, ETransactionType::Killstreak);

	TArray<const UTransaction*> OutResult_C = UGameStateTransactionHistory::Static_GetAllTransactionsOfType(World, TestActorUniqueId, ETransactionType::Killstreak);
	UTEST_EQUAL("Post-Removal, ETransactionType::Killstreak Count.", OutResult_C.Num(), 0)

	UGameStateTransactionHistory::Static_RemoveAllTransactions(World, TestActor);

	TArray<const UTransaction*> OutResult_D = UGameStateTransactionHistory::Static_GetAllTransactions(World, TestActorUniqueId);
	UTEST_EQUAL("Post-Removal, All Transaction Count.", OutResult_D.Num(), 0)

	// @gdemers cleanup.
	TestComponent->DestroyComponent();
	GameState->Destroy();
	World->DestroyWorld(true);
#endif
	return true;
}
