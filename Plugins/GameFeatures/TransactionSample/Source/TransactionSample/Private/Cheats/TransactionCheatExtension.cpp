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
#include "Cheats/TransactionCheatExtension.h"

#include "AVVMGameplay.h"
#include "AVVMTokenizer.h"
#include "GameStateTransactionHistory.h"
#include "TransactionFactoryUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#if WITH_AVVM_DEBUGGER
#include "Containers/StringFwd.h"
#include <imgui.h>
#endif

/**
 *	A Namespace specific to Testable derived types. 
 */
namespace NSTest
{
	/**
	 *	Class description:
	 *
	 *	FTransactionPayloadTest is an example Payload derive type with a single property int32.
	 */
	struct FTransactionPayloadTest : FTransactionPayload
	{
		FTransactionPayloadTest() = default;

		explicit FTransactionPayloadTest(const int32 NewDummyProperty)
			: DummyProperty(NewDummyProperty)
		{
		}

		virtual TInstancedStruct<FTransactionPayload> Init(const FString& NewPayload) override
		{
			*this = FTransactionPayloadTest(UAVVMTokenizer::GetTokenValue<int32>(TEXT("DummyProperty"), NewPayload));
			return TInstancedStruct<FTransactionPayload>::Make(*this);
		}

		virtual FString ToString() const override
		{
			FStringFormatNamedArguments Args;
			Args.Add(TEXT("DummyProperty"), FStringFormatArg{DummyProperty});
			return FString::Format(TEXT("DummyProperty:{DummyProperty}"), Args);
		}

		int32 DummyProperty = INDEX_NONE;
	};

	/**
	 *	Class description:
	 *
	 *	FTransactionFactoryImplTest is an example Factory Payload derive type that instance a FTransactionPayloadTest.
	 */
	struct FTransactionFactoryImplTest : FTransactionFactoryImpl
	{
		virtual TInstancedStruct<FTransactionPayload> CreatePayload(const FString& NewPayload) const override
		{
			auto Instanced = FTransactionPayloadTest();
			return Instanced.Init(NewPayload);
		}
	};
};

void UTransactionCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Registering %s"),
	       *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this);
#endif

	GameInstanceDelegateHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UTransactionCheatExtension::OnStartGameInstance);

	ClearTransactionHistory();
}

void UTransactionCheatExtension::RemovedFromCheatManager_Implementation()
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Unregistering %s"),
	       *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().RemoveDescriptor(this);
#endif

	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceDelegateHandle);

	ClearTransactionHistory();
}

void UTransactionCheatExtension::RemoveAllTransactionsOfType(const ETransactionType NewType, const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Remove All Transactions of type \"%s\" from Player Index \"%s\"."),
	       EnumToString(NewType),
	       *FString::FromInt(PlayerIndex));

	UGameStateTransactionHistory* TransactionComponent = TransactionHistory.Get();
	if (ensureAlwaysMsgf(IsValid(TransactionComponent), TEXT("Invalid Transaction History Component!")))
	{
		const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
		TransactionComponent->RemoveAllOfType(PlayerState, NewType);
	}
}

void UTransactionCheatExtension::RemoveAllTransactions(const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Remove All Transactions from Player Index \"%s\"."),
	       *FString::FromInt(PlayerIndex));

	UGameStateTransactionHistory* TransactionComponent = TransactionHistory.Get();
	if (ensureAlwaysMsgf(IsValid(TransactionComponent), TEXT("Invalid Transaction History Component!")))
	{
		const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
		TransactionComponent->RemoveAll(PlayerState);
	}
}

void UTransactionCheatExtension::AddTransaction(const ETransactionType NewType, const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Add Transaction of type \"%s\" to Player Index \"%s\"."),
	       EnumToString(NewType),
	       *FString::FromInt(PlayerIndex));

	UGameStateTransactionHistory* TransactionComponent = TransactionHistory.Get();
	if (ensureAlwaysMsgf(IsValid(TransactionComponent), TEXT("Invalid Transaction History Component!")))
	{
		const auto InstancedPayload = TInstancedStruct<NSTest::FTransactionPayloadTest>::Make(StaticCast<int32>(NewType));
		const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
		const FString Payload = UTransactionFactoryUtils::CreateStringPayload(InstancedPayload);
		TransactionComponent->CreateAndRecordTransaction(nullptr, PlayerState, NewType, Payload);
	}
}

void UTransactionCheatExtension::PrintAll(const ETransactionType NewType, const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Print All Transactions \"%s\" from Player Index \"%s\"."),
	       EnumToString(NewType),
	       *FString::FromInt(PlayerIndex));

	UGameStateTransactionHistory* TransactionComponent = TransactionHistory.Get();
	if (!ensureAlwaysMsgf(IsValid(TransactionComponent), TEXT("Invalid Transaction History Component!")))
	{
		return;
	}

	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const FUniqueNetIdRepl& UniqueNetId = PlayerState->GetUniqueId();
	if (!ensureAlwaysMsgf(UniqueNetId != nullptr, TEXT("Invalid FUniqueNetIdRepl from \"%s\"."), *PlayerState->GetName()))
	{
		return;
	}

	int32 Count = 0;
	for (const UTransaction* Transaction : TransactionComponent->GetAllTransactionsOfType(UniqueNetId->ToString(), NewType))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("T%s\n\t%s."),
		       *FString::FromInt(++Count),
		       *Transaction->ToString());
	}
}

#if WITH_AVVM_DEBUGGER
void UTransactionCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [Transaction]"))
	{
		return;
	}

	static int PlayerIndex = 0;

	{
		static const char* const PlayerIndexTitle = "Player Index";
		if (ImGui::InputInt(PlayerIndexTitle, &PlayerIndex))
		{
			static constexpr int MaxNumPlayers = 4;
			PlayerIndex = FMath::Clamp(PlayerIndex, 0, MaxNumPlayers);
		}

		ImGui::Dummy({ImGui::GetContentRegionAvailWidth(), 0});
	}

	const char* const TransactionTypes = LazyGatherTransactionTypes();

	{
		ImGui::Text("Transaction History");
		ImGui::Separator();

		ImGui::BeginGroup();

		static int32 CurrentTransactionTypeIndex = 0;
		static const char* const TransactionTypeTitle = "Transaction Type";
		ImGui::Combo(TransactionTypeTitle, &CurrentTransactionTypeIndex, TransactionTypes);

		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			const auto Value = StaticCast<ETransactionType>(CurrentTransactionTypeIndex);
			AddTransaction(Value, PlayerIndex);
		}

		ImGui::SameLine();

		if (ImGui::Button("RemoveAll"))
		{
			RemoveAllTransactions(PlayerIndex);
		}

		ImGui::SameLine();

		if (ImGui::Button("RemoveAllOfType"))
		{
			const auto Value = StaticCast<ETransactionType>(CurrentTransactionTypeIndex);
			RemoveAllTransactionsOfType(Value, PlayerIndex);
		}

		ImGui::SameLine();

		if (ImGui::Button("Print All"))
		{
			const auto Value = StaticCast<ETransactionType>(CurrentTransactionTypeIndex);
			PrintAll(Value, PlayerIndex);
		}

		ImGui::SameLine();

		ImGui::Dummy({ImGui::GetContentRegionAvailWidth(), 0});

		ImGui::EndGroup();
	}
}

const char* UTransactionCheatExtension::LazyGatherTransactionTypes()
{
	static TAnsiStringBuilder<512> StringBuilder;
	static bool bWasInitialized = false;

	if (!bWasInitialized)
	{
		const int32 Max = StaticCast<int32>(ETransactionType::Max);
		for (int32 i = 1 /*exclude ETransactionType::None*/; i < Max; ++i)
		{
			const FString Enum = EnumToString(StaticCast<ETransactionType>(i));
			StringBuilder.Append(Enum.GetCharArray());
			StringBuilder.Append("\0"/*enforce null termination between entries*/);
		}

		bWasInitialized = true;
	}

	return *StringBuilder;
}
#endif

void UTransactionCheatExtension::OnStartGameInstance(UGameInstance* Game)
{
	ClearTransactionHistory();

	if (!IsValid(Game))
	{
		return;
	}

	AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
	if (IsValid(GameState))
	{
		TransactionHistory = GameState->GetComponentByClass<UGameStateTransactionHistory>();
	}
}

void UTransactionCheatExtension::ClearTransactionHistory()
{
	TransactionHistory.Reset();
}
