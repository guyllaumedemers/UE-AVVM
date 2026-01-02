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

class UDataRegistry;

AVVMGAMEPLAY_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplay, Log, All);

/**
 *	Plugin Description :
 *
 *	AVVMGameplay is a CORE system that setup requirements so to properly use the AVVM plugin and it's features.
 *	It expects GameFeatures to target relevant Actors defined here, and dynamically add content :
 *
 *		Example :
 *
 *			* GFP_AddComponent<UAVVMModelComponent> -> AVVMPlayerState
 *			* GFP_AddDataMetaSources
 *			* etc...
 *
 *		Most important here for our UI system is pushing the UAVVMModelComponent Class, defined in BP, and allow the creation of
 *		our UAVVMPresenter Object who respond to the AVVM Notification system.
 *
 *	Additionally,
 *
 *	This system offers a base for any Data definition types, favoring Data driven systems, and a Resource Manager system that will
 *	handle Async loading of your resources (for any AModularActor type).
 *
 *	It defines it's own AbilitySystemComponent and InputMapping Component for handling OnPawnPosses situation where Abilities and InputMappingContext
 *	require a full refresh in respond to gameplay.
 *
 *	IMPORTANT :
 *
 *	Be aware that these actors will be available on both : Server and Client! Presenters dynamically added via the GPF_AddComponent will however
 *	not be present on the Server! (if configured properly)
 */

class FAVVMGameplayModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static UDataRegistry* GetSetAutomatedTestDataRegistry();

	static AVVMGAMEPLAY_API TSharedRef<IConsoleVariable> GetCVarEnableTickSchedulerSubsystem();
	static AVVMGAMEPLAY_API TSharedRef<IConsoleVariable> GetCVarEnableOverrideTickSchedulerRule();
	static AVVMGAMEPLAY_API TSharedRef<IConsoleVariable> GetCVarOverrideTickSchedulerJobAllotment();
	static AVVMGAMEPLAY_API TSharedRef<IConsoleVariable> GetCVarOverrideTickSchedulerTickRate();
	static AVVMGAMEPLAY_API TSharedRef<IConsoleVariable> GetCVarOverrideTickSchedulerResetTimeJobQueuePriority();

private:
	static AVVMGAMEPLAY_API TSharedPtr<IConsoleVariable> CVarEnableTickSchedulerSubsystem;
	static AVVMGAMEPLAY_API TSharedPtr<IConsoleVariable> CVarEnableOverrideTickSchedulerRule;
	static AVVMGAMEPLAY_API TSharedPtr<IConsoleVariable> CVarOverrideTickSchedulerJobAllotment;
	static AVVMGAMEPLAY_API TSharedPtr<IConsoleVariable> CVarOverrideTickSchedulerTickRate;
	static AVVMGAMEPLAY_API TSharedPtr<IConsoleVariable> CVarOverrideTickSchedulerResetTimeJobQueuePriority;

#if !UE_BUILD_SHIPPING
	FDelegateHandle HandleA;
	FDelegateHandle HandleB;
	FDelegateHandle HandleC;
	FDelegateHandle HandleD;
#endif
};
