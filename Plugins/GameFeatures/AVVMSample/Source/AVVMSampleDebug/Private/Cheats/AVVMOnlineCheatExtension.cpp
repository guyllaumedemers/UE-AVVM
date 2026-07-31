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
#include "Cheats/AVVMOnlineCheatExtension.h"

#include "AVVMGameplayModule.h"
#include "AVVMGameSession.h"
#include "AVVMLogger.h"
#include <imgui.h>

#include "AVVMPlayerState.h"
#include "OnlineSubsystemTypes.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UAVVMOnlineCheatExtension::AddedToCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Adding %s."),
	                *GetNameSafe(UAVVMOnlineCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().AddDescriptor(this);
}

void UAVVMOnlineCheatExtension::RemovedFromCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Removing %s."),
	                *GetNameSafe(UAVVMOnlineCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().RemoveDescriptor(this);
}

void UAVVMOnlineCheatExtension::MakePlayerProfile(const int32 PlayerIndex)
{
	// @gdemers TODO Add same Inventory, and other requirements to all new profile entries. 
	FAVVMPlayerProfile NewProfile
	{
			FMath::Rand()/*rand unique id to globally identify this Profile*/,
			FString::Printf(TEXT("Player%d"), PlayerIndex)
	};

	const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	AAVVMGameSession::Static_MakePlayerProfileId(this, PlayerState, NewProfile);
}

void UAVVMOnlineCheatExtension::MakePlayerPreset(const int32 PlayerIndex)
{
	static int32 GlobalIdOffset = INDEX_NONE;
	FAVVMPlayerPreset NewPreset
	{
			FMath::Rand()/*rand unique id to globally identify this Preset*/,
			FString::Printf(TEXT("PlayerPreset%d_%d"), PlayerIndex, ++GlobalIdOffset)
	};

	const APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	AAVVMGameSession::Static_MakePlayerPresetId(this, PlayerState, NewPreset);
}

void UAVVMOnlineCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [Online]"))
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
		ImGui::Text("Player Connection");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Create Player Profile")) { SERVER_EXECUTE_FORMATED_CHEAT("MakePlayerProfile %d", PlayerIndex); }

		ImGui::SameLine();
		
		if (ImGui::Button("Create Player Preset")) { SERVER_EXECUTE_FORMATED_CHEAT("MakePlayerPreset %d", PlayerIndex); }
		
		ImGui::BeginGroup();

		TArray<FString> OutProfiles;
		TArray<FString> OutPresets;
		GetPayloads(OutProfiles, OutPresets);

		static int32 CurrProfileId = 0;
		const TStringView<char> Profiles = GetProfiles(OutProfiles);

		if (ImGui::Combo("Player Profiles", &CurrProfileId, Profiles.GetData()))
		{
		}

		static int CurrItem = 0;
		const TStringView<char> Presets = GetPresets(OutPresets);

		// if (ImGui::ListBox("Game Session", &CurrItem, ANSI_ListOptions.Get()))
		// {
		// }

		ImGui::EndGroup();
		ImGui::EndGroup();
	}
}

void UAVVMOnlineCheatExtension::GetPayloads(TArray<FString>& OutProfiles,
                                            TArray<FString>& OutPresets) const
{
	const auto* GameState = UGameplayStatics::GetGameState(this);
	if (!IsValid(GameState))
	{
		return;
	}

	OutProfiles.Reset();
	OutPresets.Reset();
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const auto* NewPlayerState = Cast<AAVVMPlayerState>(PlayerState);
		if (!IsValid(NewPlayerState))
		{
			continue;
		}

		OutProfiles.Add(NewPlayerState->ClientSidedPlayerProfilePayload);
		OutPresets.Add(NewPlayerState->ClientSidedPlayerPresetPayload);
	}
}

TStringView<char> UAVVMOnlineCheatExtension::GetProfiles(const TArray<FString>& Profiles) const
{
	static TAnsiStringBuilder<512> Output{};
	Output.Reset();
	
	for (const auto& Payload : Profiles)
	{
		Output.Append(Payload.GetCharArray());
		Output.Append("\0");
	}

	return Output;
}

TStringView<char> UAVVMOnlineCheatExtension::GetPresets(const TArray<FString>& Presets) const
{
	static TAnsiStringBuilder<512> Output{};
	Output.Reset();
	
	for (const auto& Payload : Presets)
	{
		Output.Append(Payload.GetCharArray());
		Output.Append("\0");
	}

	return Output;
}
