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

#include "AVVM.generated.h"

AVVM_API DECLARE_LOG_CATEGORY_EXTERN(LogUI, Log, All);

/**
 *	Plugin Description :
 *
 *	AVVM is a CORE system that setup strong foundation for UI development. It offers a set of tools for handling View Model CRUD principles
 *	in an elegant fashion and expose a flexible 'Notification' system for broadcasting events data to your UI.
 *
 *		Snippet :
 *
 *			UE-AVVM. Actor, View, ViewModel. Based on the RAII principle, define an Actor on which we add runtime components using Unreal GameFeaturePlugin.
 *			Our runtime component, AVVMComponent, expose a collection of Presenter UObject that define the architecture of a single/multiplayer game.
 *			It's lifecycle depends on it's owning typed Outer! Each Presenter UObject, AVVMPresenter is responsible for it's own Manual ViewModel instance
 *			and is ref-count.
 *			
 *			See Full description here! (https://github.com/guyllaumedemers/UE-AVVM)
 *
 *		Class Recap :
 *
 *			* UAVVMModelComponent is a UActorComponent type, pushed via GFP, on a Target Actor Class. It holds a set of TSubClassOf<UAVVMPresenter> UObject and handle CRUD principle for these objects.
 *			* UAVVMPresenter is a UObject type that's owned by a UAVVMModelComponent. It's define the View Model Class to be instanced and store a Pair<AActor, UMVVMViewModelBase>,
 *			mapping it's Outer Actor Type to the View Model instance, with the UAVVMSubsystem, until destroyed. The View Model instanced is 'Manually' bound using the IAVVMViewModelFNameHelper UINTERFACE().
 *			* UAVVMSubsystem is a World Subsystem that manage caching ViewModel instance and allow retrieval of said instance using the AActor type provided. This imply that View Model are owned by a unique
 *			AActor which is great for situation where OnPossesChanges require displaying a new set of data to the HUD but may revert back to an existing Pawn later.
 */

// ---------------------------------------------------------------------------------------------------------------------//
//													Other Utilities														//
// ---------------------------------------------------------------------------------------------------------------------//

/**
 *	Class description:
 *
 *	UAVVMViewModelFNameHelper. Provide the ViewModel subsystem an abstract approach to binding a ViewModel name to
 *	a Widget.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVM_API UAVVMViewModelFNameHelper : public UInterface
{
	GENERATED_BODY()
};

class AVVM_API IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	virtual FName GetViewModelFName() const PURE_VIRTUAL(GetViewModelFName, return NAME_None;);
};
