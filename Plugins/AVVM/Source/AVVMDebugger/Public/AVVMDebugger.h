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

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "ImGuiModuleProperties.h"

class UGameInstance;

/**
 *	Class Description :
 *
 *	FAVVMImGuiDebuggerContext represent the Context Object which will manage drawing our debugger.
 */
struct FAVVMImGuiDebuggerContext
{
	FAVVMImGuiDebuggerContext() = default;
	FAVVMImGuiDebuggerContext(FImGuiModuleProperties& InProperties);
	void Draw();

private:
	FImGuiModuleProperties* Properties = nullptr;

	/**
	 *	Class Description :
	 *
	 *	FAVVMImGuiDebuggerProperties define a set of properties that work along the existing FImGuiModuleProperties. This separation
	 *	is to facilitate handling of both system as we most-likely don't want possible overlap to happen while running our plugin.
	 */
	struct FAVVMImGuiDebuggerProperties
	{
		bool CanPresent(FImGuiModuleProperties* ImGuiProperties) const { return (ImGuiProperties != nullptr) && !ImGuiProperties->ShowDemo() && bShowAVVMDebugger; };
		bool bShowAVVMDebugger = false;
	};

	// @gdemers instance properties
	FAVVMImGuiDebuggerProperties CustomProperties;
};

/**
 *	Plugin Description :
 *
 *	FAVVMDebuggerModule setup the base requirement to hook onto the ImGui plugin. Additional features will be added so Cheats register, and get exposed
 *	here!
 */
class FAVVMDebuggerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OnStartGameInstance(UGameInstance* Game);
	void RegisterImGuiDelegates();
	void ClearImGuiDelegates();
	// @gdemers imgui is a Immediate mode rendering library which render on tick
	// as such, application tick will execute this call which we can use to render our slate
	// widget.
	void OnWorldDrawDebug();

	FAVVMImGuiDebuggerContext AVVMDebuggerContext;
	FDelegateHandle GameInstanceDelegateHandle;
	FDelegateHandle ImGuiDelegateHandle;
};
