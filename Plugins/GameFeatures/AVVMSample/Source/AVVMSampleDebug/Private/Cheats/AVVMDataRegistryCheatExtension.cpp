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
#include "Cheats/AVVMDataRegistryCheatExtension.h"

#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include <imgui.h>
#include "DataRegistrySubsystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UAVVMDataRegistryCheatExtension::AddedToCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
					nullptr,
					this,
					TEXT("Adding %s."),
					*GetNameSafe(UAVVMDataRegistryCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().AddDescriptor(this);
}

void UAVVMDataRegistryCheatExtension::RemovedFromCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
					nullptr,
					this,
					TEXT("Removing %s."),
					*GetNameSafe(UAVVMDataRegistryCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().RemoveDescriptor(this);
}

void UAVVMDataRegistryCheatExtension::DumpRegistryTypeSummary() const
{
	UDataRegistrySubsystem::DumpRegistryTypeSummary();
}

void UAVVMDataRegistryCheatExtension::DumpCachedItems(const FString& DataRegistryType) const
{
	if (IsValid(GEngine))
	{
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("DataRegistry.DumpCachedItems %s"), *DataRegistryType));
	}
}

void UAVVMDataRegistryCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [DataRegistry]"))
	{
		return;
	}

	{
		ImGui::Text("DEBUG COMMAND");
		ImGui::Separator();

		ImGui::BeginGroup();

		if (ImGui::Button("DumpRegistryTypeSummary")) { SERVER_EXECUTE_CHEAT("DumpRegistryTypeSummary"); }

		static char str0[128] = "Input Registry Type!";
		ImGui::InputText("DataRegistry Type", str0, IM_ARRAYSIZE(str0));

		const auto Result = StringCast<TCHAR>(str0);
		if (ImGui::Button("DumpCachedItems")) { SERVER_EXECUTE_FORMATED_CHEAT("DumpCachedItems %s", Result.Get()); }

		ImGui::EndGroup();
	}
}
