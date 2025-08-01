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

class UAVVMOnlineStringParser;

AVVMONLINE_API DECLARE_LOG_CATEGORY_EXTERN(LogOnline, Log, All);

/**
 *	Plugin Description :
 *
 *	AVVMOnline is a system that offer a broad interface for general interaction with backend services. It takes advantages of the AVVM Notification
 *	system and Receive/Send type-erase context data to any endpoint using FInstancedStruct/TInstancedStruct<T>.
 *
 *	Important : Plugin requirements were created out based on Overwatch 2, and are expected to be extended to interface with your project services.
 *	ImGui has been added to support quick iterative debugging. Through the use of CVAR and the ImGui window, users can invoke registered 'Descriptors'.
 *	See AVVMDebugger for information about how to invoke the context window and AVVMSampleRuntime for project implementation examples. (Bonus)
 *	-> I've made it so symbols are stripped out in UE_SHIPPING_BUILD!
 *
 *	Additionally,
 *
 *	Here's an exhaustive list of relevant types available in the plugin for your everyday use. Note that we expect
 *	the usage of a Parser object to properly convert from FString->YourType and to YourType->FString.
 *
*		Types :
 *
 *			* FAVVMCurrency
 *			* FAVVMPlayerWallet
 *			* FAVVMPlayerResource
 *			* FAVVMPlayerChallenge
 *			* FAVVMPlayerPreset
 *			* FAVVMPlayerProfile
 *			* FAVVMPlayerAccount
 *			* FAVVMHostConfiguration
 *			* FAVVMPlayerConnection
 *			* FAVVMParty
 *			* FAVVMPlayerRequest
 *			* FAVVMLoginContext
 *
 *		* See AVVMOnlineInterface.h for a rundown of the available system interfaces.
 *
 *	Finally,
 *
 *	The exposed api has been crafted with the intent of speeding up the initial 'setup' process by which we configure the architecture of
 *	a new project but may obviously be incomplete. Feel free to extend the existing api! Using this should however still provide you with a good jump start
 *	for crafting your UI system and fleshing out the game loop!
 */
class FAVVMOnlineModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static AVVMONLINE_API TSharedRef<IConsoleVariable> GetCVarOnlineRequestReturnedStatus();
	static AVVMONLINE_API UAVVMOnlineStringParser* GetJsonParser();

private:
	// cvar that allow user testing. returns the status of a stub request (true/false) without an actual backend hooked.
	static AVVMONLINE_API TSharedPtr<IConsoleVariable> CVarOnlineRequestReturnedStatus;
	static AVVMONLINE_API TStrongObjectPtr<UAVVMOnlineStringParser> JsonParser;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FAVVMOnlineResquestDelegate, const bool /*bWasSuccess*/, const TInstancedStruct<FAVVMNotificationPayload>& /*Payload*/);

#if !UE_BUILD_SHIPPING
#define UE_AVVM_ONLINE_DEBUGGER_ENABLED 1

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
#else
#define UE_AVVM_ONLINE_DEBUGGER_ENABLED 0
#endif

#if UE_AVVM_ONLINE_DEBUGGER_ENABLED
#define AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback) FAVVMScopedDebugger ScopedDebugger(Callback);
#else
#define AVVM_EXECUTE_ONLINE_SCOPED_DEBUGLOG(Callback)
#endif
