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

void UAVVMGameplayPlayerStateCheatExtension::Die(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		// TODO @gdemers would be nice if we could inject a Proxy Component that run the desired logic
		QuickTimeEventHandler->Die(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Raise(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Raise(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Stun(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Stun(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::Exhaust(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->Exhaust(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StartCasting(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StartCasting(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StopCasting(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StopCasting(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::EarnMoney(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->EarnMoney(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::SpendMoney(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->SpendMoney(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StartInteractingWithWorld(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StartInteractingWithWorld(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StopInteractingWithWorld(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StopInteractingWithWorld(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::ConsumeItem(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->ConsumeItem(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StartTalkingToNpc(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StartTalkingToNpc(nullptr);
	}
}

void UAVVMGameplayPlayerStateCheatExtension::StopTalkingToNpc(const int32 PlayerIndex)
{
	auto QuickTimeEventHandler = TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(UGameplayStatics::GetPlayerState(this, PlayerIndex));
	if (UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler))
	{
		QuickTimeEventHandler->StopTalkingToNpc(nullptr);
	}
}

#if WITH_AVVM_DEBUGGER
void UAVVMGameplayPlayerStateCheatExtension::Draw()
{
}
#endif
