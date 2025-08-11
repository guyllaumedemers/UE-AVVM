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
#include "UObject/Interface.h"

#include "AVVMDebugger.generated.h"

class UAVVMDebuggerInputHandler;
class UGameInstance;

// @gdemers this handle IAVVMImGuiDescriptor conditional existence, as the AVVMDebugger Module wont be delivered in UE_SHIPPING_BUILD,
// and doesn't exist when Target doesn't compile with DeveloperTool included.
#ifdef UE_ENABLE_AVVM_DEBUGGER
#define WITH_AVVM_DEBUGGER 1
#else
#define WITH_AVVM_DEBUGGER 0
#endif

/**
*	Class description:
*
*	UAVVMImGuiDescriptor is an interface to be implemented by CheatExtensions and allow drawing ImGui content on screen.
*	Important : Make sure to conditionally include #include "AVVMDebugger.h" wrapped with #ifdef UE_ENABLE_AVVM_DEBUGGER in your header file,
*	and wrap the interface with #if WITH_AVVM_DEBUGGER so symbols can be properly stripped out when required.
*
*	See AVVMSampleRuntime/Cheats for examples!
*/
UINTERFACE(BlueprintType, Blueprintable)
class AVVMDEBUGGER_API UAVVMImGuiDescriptor : public UInterface
{
	GENERATED_BODY()
};

class AVVMDEBUGGER_API IAVVMImGuiDescriptor
{
	GENERATED_BODY()

public:
	virtual void Draw() PURE_VIRTUAL(Draw, return;);
};

/**
 *	Class Description :
 *
 *	FAVVMImGuiDebugContext is the context object that draw our debugger.
 */
struct AVVMDEBUGGER_API FAVVMImGuiDebugContext
{
	FAVVMImGuiDebugContext() = default;
	FAVVMImGuiDebugContext(FImGuiModuleProperties& InProperties);
	~FAVVMImGuiDebugContext();

	void AddDescriptor(const TScriptInterface<IAVVMImGuiDescriptor>& Descriptor);
	void RemoveDescriptor(const TScriptInterface<IAVVMImGuiDescriptor>& Descriptor);
	void Draw();
	void ToggleDebugger();

private:
	// @gdemers imgui properties handling inputs and demo
	FImGuiModuleProperties* Properties = nullptr;

	/**
	 *	Class Description :
	 *
	 *	FAVVMImGuiDebugProperties define a set of properties that work along the existing FImGuiModuleProperties. This separation
	 *	is to facilitate handling of both system as we most-likely don't want possible overlap to happen while running our plugin.
	 */
	struct FAVVMImGuiDebugProperties
	{
		bool CanPresent(const FImGuiModuleProperties* ImGuiProperties) const { return (ImGuiProperties != nullptr) && !ImGuiProperties->ShowDemo() && bShowDebugContext; };
		bool bShowDebugContext = false;
	};

	// @gdemers plugin properties.
	FAVVMImGuiDebugProperties CustomProperties;

	// @gdemers collection type of whats to be rendered in immediate mode.
	TArray<TScriptInterface<IAVVMImGuiDescriptor>> Descriptors;
};

/**
 *	Plugin Description :
 *
 *	AVVMDebugger is a system that configures the requirements for hooking into the ImGui plugin. It fixes some of the issues
 *	that the UE ImGui plugin has with input support by hijacking Platform inputs using a custom InputPreprocessor.
 */
class FAVVMDebuggerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	AVVMDEBUGGER_API static FAVVMDebuggerModule& Get();
	AVVMDEBUGGER_API static bool IsAvailable();
	AVVMDEBUGGER_API FAVVMImGuiDebugContext& GetDebuggerContext();

private:
	void OnStartGameInstance(UGameInstance* Game);
#if WITH_EDITOR
	void OnPIEStart(UGameInstance* Game);
	void OnPIEEnd(UGameInstance* Game);
#endif
	void RegisterImGuiDelegates();
	void ClearImGuiDelegates();
	void CreateInputHandler();
	void ClearInputHandler();
	// @gdemers imgui is a Immediate mode rendering library which render on tick
	// as such, application tick will execute this call which we can use to render our ImGui Context Class
	void OnMultiContextDebug();
	void OnToggleDebugContext();

	// @gdemers the purpose of this input handler workaround issue created by the ImGuiInputHandler.
	// Problem : Until you invoke from console the ToggleInput cheat, the visibility state of the SImGuiWidget won't be set 'Visible'
	// and instead is default to 'HitTestInvisible'. By doing so, they basically prevent any input from being processed
	// and whatever key is bound in Project settings cannot be invoked.
	TStrongObjectPtr<UAVVMDebuggerInputHandler> InputHandler = nullptr;

	FAVVMImGuiDebugContext DebugContext;
	FDelegateHandle GameInstanceDelegateHandle;
	FDelegateHandle PIEStartDelegateHandle;
	FDelegateHandle PIEEndDelegateHandle;
	FDelegateHandle ImGuiDelegateHandle;
};
