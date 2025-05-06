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

#include "AVVMDebugger.h"

#include "ImGuiDelegates.h"
#include "ImGuiModule.h"
#include "Engine/GameInstance.h"

FAVVMImGuiDebuggerContext::FAVVMImGuiDebuggerContext(FImGuiModuleProperties& InProperties)
{
	Properties = &InProperties;
}

void FAVVMImGuiDebuggerContext::Draw()
{
	const bool bCanShowDebugger = CustomProperties.CanPresent(Properties);
	if (!bCanShowDebugger)
	{
		return;
	}

	{
		ImGui::Text("Hello, world!");
	}
}

void FAVVMDebuggerModule::StartupModule()
{
	// @gdemers I really don't like this approach. I feel like the ImGui plugin shouldn't hide the FImGuiModuleManager in the private access modifier.
	// Doing so prevent any third party plugin from being made aware of the FImGuiContextProxy tied to a given ContextIndex/World
	// and prevent directly binding to the FImGuiContextProxy::OnDraw delegate. (which is really the only thing I care about being hooked to here!)
	// Even if they exposed a set of Delegates to their api, it cannot be bound to in the Startup function due to the call to Utilities::GetWorldContextIndex
	// returning INVALID_INDEX and FImGuiDelegatesContainer::WorldDebugDelegates.FindOrAdd failing to find a valid Key. (all due to the World not being yet created!)
	GameInstanceDelegateHandle = FWorldDelegates::OnStartGameInstance.AddRaw(this, &FAVVMDebuggerModule::OnStartGameInstance);
}

void FAVVMDebuggerModule::ShutdownModule()
{
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceDelegateHandle);
}

void FAVVMDebuggerModule::OnStartGameInstance(UGameInstance* Game)
{
	RegisterImGuiDelegates();
}

void FAVVMDebuggerModule::RegisterImGuiDelegates()
{
	if (!ImGuiDelegateHandle.IsValid())
	{
		ImGuiDelegateHandle = FImGuiDelegates::OnWorldEarlyDebug().AddRaw(this, &FAVVMDebuggerModule::OnWorldDrawDebug);
	}

	const bool bIsAvailable = FImGuiModule::IsAvailable();
	if (bIsAvailable)
	{
		AVVMDebuggerContext = FAVVMImGuiDebuggerContext(FImGuiModule::Get().GetProperties());
	}
}

void FAVVMDebuggerModule::ClearImGuiDelegates()
{
	FImGuiDelegates::OnWorldEarlyDebug().Remove(ImGuiDelegateHandle);
}

void FAVVMDebuggerModule::OnWorldDrawDebug()
{
	AVVMDebuggerContext.Draw();
}

IMPLEMENT_MODULE(FAVVMDebuggerModule, AVVMDebugger)
