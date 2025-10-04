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
#include "Cheats/AVVMGameplayPlayerStateCheatExtension.h"

#include "AVVM.h"
#include "AVVMGameplaySampleSettings.h"
#include "AVVMQuicktimeEventInterface.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#if WITH_AVVM_DEBUGGER
#include "Containers/StringFwd.h"
#include <imgui.h>
#endif

void UAVVMGameplayPlayerStateCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Registering %s"), *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this);
#endif
}

void UAVVMGameplayPlayerStateCheatExtension::RemovedFromCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Unregistering %s"), *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().RemoveDescriptor(this);
#endif
}

void UAVVMGameplayPlayerStateCheatExtension::Damage(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Damage(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetHealthComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Heal(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Heal(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetHealthComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Die(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Die(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetHealthComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Raise(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler),
	                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
	{
		QuickTimeEventHandler->Raise(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetHealthComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Stun(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Stun(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetStatusEffectComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Exhaust(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Exhaust(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetStaminaComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StartCasting(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler),
	                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
	{
		QuickTimeEventHandler->StartCasting(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetAbilityComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StopCasting(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StopCasting(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetAbilityComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::EarnMoney(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->EarnMoney(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetCurrencyComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::SpendMoney(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->SpendMoney(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetCurrencyComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StartInteractingWithWorld(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StartInteractingWithWorld(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetInteractionComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StopInteractingWithWorld(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StopInteractingWithWorld(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetInteractionComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::ConsumeItem(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->ConsumeItem(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetInteractionComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StartTalkingToNpc(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StartTalkingToNpc(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetInteractionComponentClass()));
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StopTalkingToNpc(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	const auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState);
	if (UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StopTalkingToNpc(Pawn->GetComponentByClass(UAVVMGameplaySampleSettings::GetInteractionComponentClass()));
	}
}

#if WITH_AVVM_DEBUGGER
void UAVVMGameplayPlayerStateCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [PlayerState]"))
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

	{
		ImGui::Text("Health");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Damage")) { Damage(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Heal")) { Heal(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Die")) { Die(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Raise")) { Raise(PlayerIndex); }

		ImGui::SameLine();

		ImGui::EndGroup();
	}

	{
		ImGui::Text("Status Effect");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Stun")) { Stun(PlayerIndex); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("Stamina");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Exhaust")) { Exhaust(PlayerIndex); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("Abilities");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Start Casting")) { StartCasting(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Stop Casting")) { StopCasting(PlayerIndex); }

		ImGui::EndGroup();
	}

	{
		// TODO @gdemers Require to be better defined! Workout details about these calls. What do they involve ?
		ImGui::Text("World Interactions");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Earn Money")) { EarnMoney(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Spend Money")) { SpendMoney(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Start Interaction")) { StartInteractingWithWorld(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Stop Interaction")) { StopInteractingWithWorld(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Consume Item")) { ConsumeItem(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Start Talking to NPC")) { StartTalkingToNpc(PlayerIndex); }

		ImGui::SameLine();

		if (ImGui::Button("Stop Talking to NPC")) { StopTalkingToNpc(PlayerIndex); }

		ImGui::EndGroup();
	}
}
#endif
