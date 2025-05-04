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

#include "AVVMNotificationSubsystem.h"
#include "StructUtils/InstancedStruct.h"

/**
 *	Plugin Description :
 *
 *	FAVVMOnlineModule define a set of utilities to be used in the GameFeature plugin Module FAVVMSampleRuntime.
 */
class FAVVMOnlineModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static AVVMONLINE_API TSharedRef<IConsoleVariable> GetCVarOnlineRequestReturnedStatus();
	static AVVMONLINE_API UObject* GetJsonParser();

private:
	/**
	 *	Variable Description :
	 *
	 *	CVar to allow user testing of 'fake' online request being made without an actual backend hooked. (Cuz thats way too much work and UCommonUser plugin
	 *	isn't so friendly)
	 */
	static AVVMONLINE_API TSharedPtr<IConsoleVariable> CVarOnlineRequestReturnedStatus;
	static AVVMONLINE_API TStrongObjectPtr<UObject> JsonParser;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FAVVMOnlineResquestDelegate, const bool /*bWasSuccess*/, const TInstancedStruct<FAVVMNotificationPayload>& /*Payload*/);

#if not defined UE_AVVM_DEBUGGER_ENABLED
#define UE_AVVM_DEBUGGER_ENABLED 0
#endif

#if !UE_BUILD_SHIPPING
#define UE_AVVM_DEBUGGER_ENABLED 1

struct FAVVMScopedDebugger
{
	FAVVMScopedDebugger() = default;

	FAVVMScopedDebugger(const FAVVMOnlineResquestDelegate& Callback)
	{
		bool bCompletionStatus;
		FAVVMOnlineModule::GetCVarOnlineRequestReturnedStatus()->GetValue(bCompletionStatus);
		Callback.Broadcast(bCompletionStatus, {});
	}
};
#endif

#if UE_AVVM_DEBUGGER_ENABLED
#define AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback) FAVVMScopedDebugger ScopedDebugger(Callback);
#else
#define AVVM_EXECUTE_SCOPED_DEBUGLOG(Callback)
#endif
