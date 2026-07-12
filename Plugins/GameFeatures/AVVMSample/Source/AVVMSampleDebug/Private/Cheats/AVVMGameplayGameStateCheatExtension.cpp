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
#include "Cheats/AVVMGameplayGameStateCheatExtension.h"

#include "AVVMLogger.h"
#include "AVVMModule.h"
#include "AVVMQuicktimeEventInterface.h"
#include "AVVMToolkitUtils.h"
#include <imgui.h>
#include "Containers/StringFwd.h"
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"

void UAVVMGameplayGameStateCheatExtension::AddedToCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Adding %s."),
	                *GetNameSafe(UAVVMGameplayGameStateCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().AddDescriptor(this);
}

void UAVVMGameplayGameStateCheatExtension::RemovedFromCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Removing %s."),
	                *GetNameSafe(UAVVMGameplayGameStateCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().RemoveDescriptor(this);
}

void UAVVMGameplayGameStateCheatExtension::Disconnect()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Disconnect();
	}
}

void UAVVMGameplayGameStateCheatExtension::Connect()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Connect();
	}
}

void UAVVMGameplayGameStateCheatExtension::Win()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Win();
	}
}

void UAVVMGameplayGameStateCheatExtension::Lose()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Lose();
	}
}

void UAVVMGameplayGameStateCheatExtension::Kill()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Kill();
	}
}

void UAVVMGameplayGameStateCheatExtension::Killstreak()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Killstreak();
	}
}

void UAVVMGameplayGameStateCheatExtension::CaptureObjective(const int32 ObjectiveId)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->CaptureObjective(ObjectiveId);
	}
}

void UAVVMGameplayGameStateCheatExtension::DiscoverArea(const int32 AreaId)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->DiscoverArea(AreaId);
	}
}

void UAVVMGameplayGameStateCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [GameState]"))
	{
		return;
	}

	{
		ImGui::Text("Online");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Disconnect")) { SERVER_EXECUTE_CHEAT("Disconnect"); }

		ImGui::SameLine();

		if (ImGui::Button("Connect")) { SERVER_EXECUTE_CHEAT("Connect"); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("After Action Report");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Lose")) { SERVER_EXECUTE_CHEAT("Lose"); }

		ImGui::SameLine();

		if (ImGui::Button("Win")) { SERVER_EXECUTE_CHEAT("Win"); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("Leaderboard");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Kill")) { SERVER_EXECUTE_CHEAT("Kill"); }

		ImGui::SameLine();

		if (ImGui::Button("Killstreak")) { SERVER_EXECUTE_CHEAT("Killstreak"); }

		ImGui::SameLine();
		
		ImGui::InputInt("Objective Id", &SelectedObjectiveId);

		ImGui::SameLine();
		
		if (ImGui::Button("CaptureObjective")) { SERVER_EXECUTE_FORMATED_CHEAT("CaptureObjective %d", SelectedObjectiveId); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("GameplayEvent");
		ImGui::Separator();

		ImGui::BeginGroup();
		
		ImGui::InputInt("Area Id", &SelectedAreaId);

		ImGui::SameLine();

		if (ImGui::Button("DiscoverArea")) { SERVER_EXECUTE_FORMATED_CHEAT("DiscoverArea %d", SelectedAreaId); }

		ImGui::EndGroup();
	}
}
