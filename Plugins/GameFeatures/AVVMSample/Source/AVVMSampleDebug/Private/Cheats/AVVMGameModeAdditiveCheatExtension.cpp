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
#include "Cheats/AVVMGameModeAdditiveCheatExtension.h"

#include "AVVMGameplayModule.h"
#include "AVVMGameModeAdditive.h"
#include "AVVMGameplaySettings.h"
#include "AVVMImGuiModule.h"
#include "AVVMLogger.h"
#include "DataRegistrySubsystem.h"
#include <imgui.h>
#include "Kismet/GameplayStatics.h"
#include "Misc/StringOutputDevice.h"

void UAVVMGameModeAdditiveCheatExtension::AddedToCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Adding %s."),
	                *GetNameSafe(UAVVMGameModeAdditiveCheatExtension::StaticClass()));

	// @gdemers hard reset between PIE sessions as the CheatExtension wasnt in memory
	// when editor changed was applied! for Runtime, we listen to the Module event so if a GFP is adding data registries or tags, we can process.
	bHasRegistriesChanged = true;

	FAVVMImGuiModule::Get().GetDebuggerContext().AddDescriptor(this);

	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (IsValid(DataRegistrySubsystem))
	{
		// @gdemers upon activating a new GFP_DataRegistry, the subsystem will broadcast this event! (each time)
		DataRegistrySubsystem->OnSubsystemInitialized().AddUObject(this, &UAVVMGameModeAdditiveCheatExtension::OnDataRegistrySubsystemChanged);
	}
}

void UAVVMGameModeAdditiveCheatExtension::RemovedFromCheatManager_Implementation()
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Removing %s."),
	                *GetNameSafe(UAVVMGameModeAdditiveCheatExtension::StaticClass()));

	FAVVMImGuiModule::Get().GetDebuggerContext().RemoveDescriptor(this);
}

void UAVVMGameModeAdditiveCheatExtension::Create(const FString& GameModeAdditiveClassAssetName)
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Creating %s."),
	                *GameModeAdditiveClassAssetName);

	UAVVMGameModeAdditiveUtils::AddOrRemoveGameModeAdditive(this, FName(GameModeAdditiveClassAssetName), true);
}

void UAVVMGameModeAdditiveCheatExtension::Destroy(const FString& GameModeAdditiveClassAssetName)
{
	AVVM_LOGGER_LOG(LogGameplay,
	                nullptr,
	                this,
	                TEXT("Destroying %s."),
	                *GameModeAdditiveClassAssetName);

	UAVVMGameModeAdditiveUtils::AddOrRemoveGameModeAdditive(this, FName(GameModeAdditiveClassAssetName), false);
}

void UAVVMGameModeAdditiveCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [GameMode Additive]"))
	{
		return;
	}
	
	const char* const GameModeAdditiveClasses = LazyGatherGameModeAdditiveClasses(bHasRegistriesChanged);
	
	{
		ImGui::Text("GameMode");
		ImGui::Separator();

		ImGui::BeginGroup();
		
		static int32 CurrentGameModeAdditiveClassIndex = 0;
		static const char* const GameModeAdditiveTitle = "AVVMGameModeAdditive";
		ImGui::Combo(GameModeAdditiveTitle, &CurrentGameModeAdditiveClassIndex, GameModeAdditiveClasses);

		ImGui::SameLine();

		ImGui::Dummy({ImGui::GetContentRegionAvailWidth(), 0});

		ImGui::EndGroup();
		
		if (ImGui::Button("Create"))
		{
			const FString GameModeAdditiveAssetName = GetIndexedString(GameModeAdditiveClasses, CurrentGameModeAdditiveClassIndex);
			SERVER_EXECUTE_FORMATED_CHEAT("Create %s", *GameModeAdditiveAssetName);
		}

		ImGui::SameLine();
		
		if (ImGui::Button("Destroy"))
		{
			const FString GameModeAdditiveAssetName = GetIndexedString(GameModeAdditiveClasses, CurrentGameModeAdditiveClassIndex);
			SERVER_EXECUTE_FORMATED_CHEAT("Destroy %s", *GameModeAdditiveAssetName);
		}
	}
}

const char* UAVVMGameModeAdditiveCheatExtension::LazyGatherGameModeAdditiveClasses(bool& bForceGathering) const
{
	static TAnsiStringBuilder<512> StringBuilder;
	static bool bWasInitialized = false;

	if (bWasInitialized && !bForceGathering)
	{
		return *StringBuilder;
	}

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return *StringBuilder;
	}

	const UDataRegistry* DataRegistry = Subsystem->GetRegistryForType(UAVVMGameplaySettings::GetGameModeAdditiveRegistryType());
	if (!IsValid(DataRegistry))
	{
		return *StringBuilder;
	}

	TArray<const FAVVMGameModeAdditiveDefinitionDataTableRow*> OutGameModeAdditiveDataTableRows;
	DataRegistry->GetAllItems(TEXT(""), OutGameModeAdditiveDataTableRows);

	for (const auto* TableRow : OutGameModeAdditiveDataTableRows)
	{
		if (TableRow != nullptr)
		{
			StringBuilder.Append(TableRow->GameModeAdditiveClass.GetAssetName().GetCharArray());
			StringBuilder.Append("\0"/*enforce null termination between entries*/);
		}
	}

	bWasInitialized = true;
	bForceGathering = false;
	// @gdemers operator* and ToString enforce null termination
	return *StringBuilder/*last entry will be \0\0 as expected by ImGui::Combo*/;
}

FString UAVVMGameModeAdditiveCheatExtension::GetIndexedString(const char* ConcatString,
                                                              const int32 Index) const
{
	int32 ReverseCount = Index;

	const char* Head = ConcatString;
	while (*Head && ReverseCount > 0)
	{
		Head += strlen(Head) + 1;
		--ReverseCount;
	}

	return FString(Head);
}

void UAVVMGameModeAdditiveCheatExtension::OnDataRegistrySubsystemChanged()
{
	bHasRegistriesChanged = true;
}
