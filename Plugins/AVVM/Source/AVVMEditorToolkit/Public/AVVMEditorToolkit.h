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

/**
 *	Class description:
 *	
 *	FAVVMEditorToolkit_Core is a derived class that impl our Tool core behaviour.
 */
class FAVVMEditorToolkit_Core : public FAssetEditorToolkit
{
public:
	FAVVMEditorToolkit_Core() = default;
	
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
};

/**
 *	Class description:
 *	
 *	FAVVMEditorToolkit_Commands is a global instance class that register ui commands / inputs with the engine input manager, and trigger
 *	events for our Editor extensions.
 */
class FAVVMEditorToolkit_Commands : public TCommands<FAVVMEditorToolkit_Commands>
{
public:
	FAVVMEditorToolkit_Commands();
	
	// TCommand<> interface
	virtual void RegisterCommands() override;
	// End of TCommand<> interface

	TSharedPtr<FUICommandInfo> OpenEditorToolkit = nullptr;
};

/**
 *	Plugin Description :
 *
 *	AVVMEditorToolkit is a utility system that define a set of systems in Slate. This plugin will NOT be packaged with your client build!
 *
 *		TODO @gdemers do a rundown of all available utilities here as you go!
 */
class FAVVMEditorToolkitModule : public IModuleInterface,
                                 public IHasMenuExtensibility,
                                 public IHasToolBarExtensibility
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	// IHasToolBarExtensibility
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override;
	// IHasMenuExtensibility
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override;

protected:
	void BindCommands();
	void RegisterUnderWindowTab();
	void RegisterUnderLevelEditorTab();
	void AppendMenuBar();

	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager = nullptr;
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager = nullptr;
	// @gdemers core object handle the toolkit cmds invocation.
	TSharedPtr<FAVVMEditorToolkit_Core> Core = nullptr;
};