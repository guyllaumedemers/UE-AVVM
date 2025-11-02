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
#include "GameStateTransactionHistory.h"
#include "TransactionFactoryUtils.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/TransactionFactoryImplTest.h"

#if WITH_AVVM_DEBUGGER
#include "Containers/StringFwd.h"
#include <imgui.h>
#endif

void UTransactionCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Registering %s"),
	       *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this);
#endif
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
}

void UTransactionCheatExtension::RemoveAllTransactionsOfType(const ETransactionType NewType, const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Remove All Transactions of type \"%s\" from Player Index \"%s\"."),
	       EnumToString(NewType),
	       *FString::FromInt(PlayerIndex));

	const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	UGameStateTransactionHistory::Static_RemoveAllTransactionOfType(this, PlayerState, NewType);
}

void UTransactionCheatExtension::RemoveAllTransactions(const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Remove All Transactions from Player Index \"%s\"."),
	       *FString::FromInt(PlayerIndex));

	const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	UGameStateTransactionHistory::Static_RemoveAllTransactions(this, PlayerState);
}

void UTransactionCheatExtension::AddTransaction(const ETransactionType NewType, const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Add Transaction of type \"%s\" to Player Index \"%s\"."),
	       EnumToString(NewType),
	       *FString::FromInt(PlayerIndex));

	const TInstancedStruct<FTransactionPayload> InputPayload = FTransactionPayload::Make<FTransactionPayloadTest>(StaticCast<int32>(NewType));

	FTransactionContextArgs Args;
	Args.Instigator = nullptr;
	Args.Target = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	Args.TransactionType = NewType;
	Args.Payload = UTransactionFactoryUtils::CreateStringPayload(InputPayload);
	
	UGameStateTransactionHistory::Static_CreateAndRecordTransaction(this, Args);
}

void UTransactionCheatExtension::PrintAll(const int32 PlayerIndex)
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Print All Transactions from Player Index \"%s\"."),
	       *FString::FromInt(PlayerIndex));

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

	for (const UTransaction* Transaction : UGameStateTransactionHistory::Static_GetAllTransactions(this, UniqueNetId->ToString()))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("%s"),
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
			const auto Value = StaticCast<ETransactionType>(CurrentTransactionTypeIndex + 1);
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
			const auto Value = StaticCast<ETransactionType>(CurrentTransactionTypeIndex + 1);
			RemoveAllTransactionsOfType(Value, PlayerIndex);
		}

		ImGui::SameLine();

		if (ImGui::Button("Print All"))
		{
			PrintAll(PlayerIndex);
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
