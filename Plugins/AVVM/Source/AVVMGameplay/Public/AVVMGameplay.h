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

AVVMGAMEPLAY_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplay, Log, All);

/**
 *	Plugin Description :
 *
 *	FAVVMGameplayModule define a set of utilities to be used in the GameFeature plugin Module FAVVMSampleRuntime. In this module, we define
 *	the base Actor Class that implement the AVVMOnline interfaces and gameplay specific interfaces.
 *
 *	IMPORTANT :
 *
 *	Be aware that these actors will be available on both : Server and Client! Presenters dynamically added via the GPF_AddComponent will however
 *	not be present on the Server! (if configured properly)
 */
