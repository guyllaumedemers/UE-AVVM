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

void UAVVMGameplayGameStateCheatExtension::CaptureObjective()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->CaptureObjective();
	}
}

void UAVVMGameplayGameStateCheatExtension::DiscoverArea()
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventGameStateInterface>(UGameplayStatics::GetGameState(this));
	if (UAVVMToolkitUtils::IsNativeScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->DiscoverArea();
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

		if (ImGui::Button("Disconnect")) { Disconnect(); }

		ImGui::SameLine();

		if (ImGui::Button("Connect")) { Connect(); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("After Action Report");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Lose")) { Lose(); }

		ImGui::SameLine();

		if (ImGui::Button("Win")) { Win(); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("Leaderboard");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("Kill")) { Kill(); }

		ImGui::SameLine();

		if (ImGui::Button("Killstreak")) { Killstreak(); }

		ImGui::SameLine();

		if (ImGui::Button("CaptureObjective")) { CaptureObjective(); }

		ImGui::EndGroup();
	}

	{
		ImGui::Text("GameplayEvent");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("DiscoverArea")) { DiscoverArea(); }

		ImGui::EndGroup();
	}
}
