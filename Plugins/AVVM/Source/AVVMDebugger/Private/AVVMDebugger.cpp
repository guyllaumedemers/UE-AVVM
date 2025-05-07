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

#include "AVVMDebuggerInputHandler.h"
#include "AVVMDebuggerSettings.h"
#include "ImGuiDelegates.h"
#include "ImGuiModule.h"
#include "Engine/GameInstance.h"

FAVVMImGuiDebugContext::FAVVMImGuiDebugContext(FImGuiModuleProperties& InProperties)
{
	Properties = &InProperties;
}

FAVVMImGuiDebugContext::~FAVVMImGuiDebugContext()
{
	if (Properties != nullptr)
	{
		// @gdemers PIE End events aren't used which make any Clear call invalid between
		// sessions. FImGuiModule::GetProperties don't have a reset function and persist between
		// PIE session. Manual handling is required.
		Properties->SetInputEnabled(false);
		Properties->SetShowDemo(false);
	}
}

void FAVVMImGuiDebugContext::AddDescriptor(const TScriptInterface<IAVVMImGuiDescriptor>& Descriptor)
{
	Descriptors.Add(Descriptor);
}

void FAVVMImGuiDebugContext::RemoveDescriptor(const TScriptInterface<IAVVMImGuiDescriptor>& Descriptor)
{
	Descriptors.Remove(Descriptor);
}

void FAVVMImGuiDebugContext::Draw()
{
	const bool bCanDraw = CustomProperties.CanPresent(Properties);
	if (!bCanDraw)
	{
		return;
	}

	const auto RenderAll = [](TArray<TScriptInterface<IAVVMImGuiDescriptor>>& OutDescriptors)
	{
		for (auto Iterator = OutDescriptors.CreateIterator(); Iterator; ++Iterator)
		{
			auto Descriptor = TScriptInterface<IAVVMImGuiDescriptor>(*Iterator);
			if (IsValid(Descriptor.GetObject()) && Descriptor.GetInterface() != nullptr)
			{
				Descriptor->Draw();
			}
			else
			{
				Iterator.RemoveCurrent();
			}
		}
	};

	static bool* bShouldCloseWindow = nullptr;
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
	ImGui::Begin("Debug Window", bShouldCloseWindow);
	RenderAll(Descriptors);
	ImGui::End();
}

void FAVVMImGuiDebugContext::ToggleDebugger()
{
	bool bIsInputEnabled = false;
	if (Properties != nullptr)
	{
		bIsInputEnabled = Properties->IsInputEnabled();
		// @gdemers we never want the debug context to be active when opening
		// this doesnt prevent invoking it when the input are already enabled, just
		// being open on Input value Changed.
		Properties->SetShowDemo(false);
	}

	CustomProperties.bShowDebugContext = bIsInputEnabled;
}

void FAVVMDebuggerModule::StartupModule()
{
	// @gdemers I really don't like this approach. I feel like the ImGui plugin shouldn't hide the FImGuiModuleManager in the private access modifier.
	// Doing so prevent any third party plugin from being made aware of the FImGuiContextProxy tied to a given ContextIndex/World
	// and prevent directly binding to the FImGuiContextProxy::OnDraw delegate. (which is really the only thing I care about being hooked to here!)
	// Even if they exposed a set of Delegates to their api, it cannot be bound to in the Startup function due to the call to Utilities::GetWorldContextIndex
	// returning INVALID_INDEX and FImGuiDelegatesContainer::WorldDebugDelegates.FindOrAdd failing to find a valid Key. (all due to the World not being yet created!)
	GameInstanceDelegateHandle = FWorldDelegates::OnStartGameInstance.AddRaw(this, &FAVVMDebuggerModule::OnStartGameInstance);
	PIEStartDelegateHandle = FWorldDelegates::OnPIEStarted.AddRaw(this, &FAVVMDebuggerModule::OnPIEStart);
	PIEEndDelegateHandle = FWorldDelegates::OnPIEEnded.AddRaw(this, &FAVVMDebuggerModule::OnPIEEnd);
}

void FAVVMDebuggerModule::ShutdownModule()
{
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceDelegateHandle);
	FWorldDelegates::OnPIEStarted.Remove(PIEStartDelegateHandle);
	FWorldDelegates::OnPIEEnded.Remove(PIEEndDelegateHandle);
	ClearImGuiDelegates();
	ClearInputHandler();
}

FAVVMDebuggerModule& FAVVMDebuggerModule::Get()
{
	return FModuleManager::LoadModuleChecked<FAVVMDebuggerModule>("AVVMDebugger");
}

bool FAVVMDebuggerModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded("AVVMDebugger");
}

FAVVMImGuiDebugContext& FAVVMDebuggerModule::GetDebuggerContext()
{
	return DebugContext;
}

void FAVVMDebuggerModule::OnStartGameInstance(UGameInstance* Game)
{
	CreateInputHandler();
	RegisterImGuiDelegates();
}

void FAVVMDebuggerModule::OnPIEStart(UGameInstance* Game)
{
	CreateInputHandler();
	RegisterImGuiDelegates();
}

void FAVVMDebuggerModule::OnPIEEnd(UGameInstance* Game)
{
	ClearInputHandler();
	ClearImGuiDelegates();
}

void FAVVMDebuggerModule::RegisterImGuiDelegates()
{
	if (!ImGuiDelegateHandle.IsValid())
	{
		ImGuiDelegateHandle = FImGuiDelegates::OnMultiContextDebug().AddRaw(this, &FAVVMDebuggerModule::OnMultiContextDebug);
	}

	const bool bIsAvailable = FImGuiModule::IsAvailable();
	if (bIsAvailable)
	{
		DebugContext = FAVVMImGuiDebugContext(FImGuiModule::Get().GetProperties());
	}
}

void FAVVMDebuggerModule::ClearImGuiDelegates()
{
	FImGuiDelegates::OnMultiContextDebug().Remove(ImGuiDelegateHandle);
}

void FAVVMDebuggerModule::CreateInputHandler()
{
	UAVVMDebuggerInputHandler* Target = InputHandler.Get();
	if (!IsValid(Target))
	{
		Target = NewObject<UAVVMDebuggerInputHandler>(GEngine, UAVVMDebuggerSettings::GetInputHandlerClass());
		InputHandler = TStrongObjectPtr<UAVVMDebuggerInputHandler>(Target);
	}

	if (ensureAlways(IsValid(Target)))
	{
		Target->SafeBegin(FSimpleDelegate::CreateRaw(this, &FAVVMDebuggerModule::OnToggleDebugContext));
	}
}

void FAVVMDebuggerModule::ClearInputHandler()
{
	UAVVMDebuggerInputHandler* Target = InputHandler.Get();
	if (ensureAlways(IsValid(Target)))
	{
		Target->SafeEnd();
	}

	const bool bIsAvailable = FImGuiModule::IsAvailable();
	if (bIsAvailable)
	{
		DebugContext = FAVVMImGuiDebugContext();
	}
}

void FAVVMDebuggerModule::OnMultiContextDebug()
{
	DebugContext.Draw();
}

void FAVVMDebuggerModule::OnToggleDebugContext()
{
	FImGuiModule::Get().GetProperties().ToggleInput();
	DebugContext.ToggleDebugger();
}

IMPLEMENT_MODULE(FAVVMDebuggerModule, AVVMDebugger)
