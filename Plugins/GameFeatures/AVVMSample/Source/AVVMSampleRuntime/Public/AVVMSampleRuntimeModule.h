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
#include "Modules/ModuleInterface.h"

/**
*	Plugin Description :
 *
 *	Runtime Sample plugin. Based on Miro board (See : https://github.com/guyllaumedemers/UE-AVVM), define a set of class
 *	that would provide boiler plate code support for generic multiplayer-game project architecture.
 */

// @gdemers expect preprocessor to be defined in build.cs, or not!
#if defined UE_AVVM_ENABLE_ONLINE_REQUEST_BRANCHING
#define UE_AVVM_CAN_HOST_ONLY_EXECUTE_ACTION 1
#else
#define UE_AVVM_CAN_HOST_ONLY_EXECUTE_ACTION 0
#endif

#if defined UE_ENABLE_AVVM_DEBUGGER
#define WITH_AVVM_DEBUGGER 1
#else
#define WITH_AVVM_DEBUGGER 0
#endif
