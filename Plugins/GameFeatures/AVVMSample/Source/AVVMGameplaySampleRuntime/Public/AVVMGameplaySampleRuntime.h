﻿//Copyright(c) 2025 gdemers
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

#include "AVVMQuicktimeEventInterface.h"

/**
 *	Plugin Description :
 *
 *	AVVMGameplaySampleRuntime is a Sandbox for testing AVVMGameplay (IAVVMQuicktimeEventGameStateInterface & IAVVMQuicktimeEventPlayerStateInterface) UINTERFACE().
 *	It offers a set of cheats from which recurrent gameplay features can be tested from.
*/

DECLARE_MULTICAST_DELEGATE_TwoParams(FAVVMOnExecuteDebugEvent, const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>&, const UActorComponent*)

#if !UE_BUILD_SHIPPING
#define UE_AVVM_GAMEPLAY_DEBUGGER_ENABLED 1

#include "GameFramework/Pawn.h"

struct FAVVMGameplayScopedDebugger
{
	FAVVMGameplayScopedDebugger() = default;

	// TODO @gdemers Define how Gameplay testing should be debugged while being able to strip out symbols in SHIPPING_BUILD.
};
#else
#define UE_AVVM_GAMEPLAY_DEBUGGER_ENABLED 0
#endif

#if UE_AVVM_GAMEPLAY_DEBUGGER_ENABLED
#define AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(WorldContextObject, PlayerIndex, ComponentClass, Callback)
#else
#define AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(WorldContextObject, PlayerIndex, ComponentClass, Callback)
#endif
