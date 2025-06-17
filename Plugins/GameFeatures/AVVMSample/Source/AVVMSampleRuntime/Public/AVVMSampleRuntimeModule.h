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
 *	AVVMSampleRuntime is a Sandbox that offer boiler plate implementation of the AVVM suite plugin. It setups the basic requirements
 *	for a multiplayer/coop game.
 *
 *	Example (Game - Overwatch 2) :
 *
 *		* See Miro link : https://github.com/guyllaumedemers/UE-AVVM
 *
 *	This suite provide a starting point to users when building their Menu-ing system. Instead of doing again and again the same architecture,
 *	use this and jump start your project!
 *
 *		Unreal Recap :
 *
 *			* Unreal's Actor Model architecture can be reduced to three points of interest. Those Actors are what are driving most systems and allow proper RAII principle for our View Model data.
 *			There is no data persistency, so no global ViewModel being used. Check project README.md and understand why!
 *
 *				A) AGameMode
 *				B) AGameStateBase
 *				C) APlayerState
 *
 *		Quick Recap : AVVM Plugin
 *
 *			* UAVVMModelComponent is a UActorComponent type, pushed via GFP, on a Target Actor Class. It holds a set of TSubClassOf<UAVVMPresenter> UObject and handle CRUD principle for these objects.
 *			* UAVVMPresenter is a UObject type that's owned by a UAVVMModelComponent. It's define the View Model Class to be instanced and store a Pair<AActor, UMVVMViewModelBase>,
 *			mapping it's Outer Actor Type to the View Model instance, with the UAVVMSubsystem, until destroyed. The View Model instanced is 'Manually' bound using the IAVVMViewModelFNameHelper UINTERFACE().
 *			* UAVVMSubsystem is a World Subsystem that manage caching ViewModel instance and allow retrieval of said instance using the AActor type provided. This imply that View Model are owned by a unique
 *			AActor which is great for situation where OnPossesChanges require displaying a new set of data to the HUD but may revert back to an existing Pawn later.
 *
 *		Recap : AVVMSampleRuntime Plugin
 *
 *		The following UINTERFACE() expose api for recurrent function call that interface with the CommonGame & UIExtension plugin for pushing Widget on screen. UAVVMPresenter should implement one of these
 *		based on system requirements.
 *			* IAVVMUIExtensionInterface
 *			* IAVVMPrimaryGameLayoutInterface
 */

// @gdemers expect preprocessor to be defined in build.cs, or not!
#ifdef UE_AVVM_ENABLE_ONLINE_REQUEST_BRANCHING
#define UE_AVVM_CAN_HOST_ONLY_EXECUTE_ACTION 1
#else
#define UE_AVVM_CAN_HOST_ONLY_EXECUTE_ACTION 0
#endif
