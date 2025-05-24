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
 *	UE-AVVM. Actor, View, ViewModel. Based on the RAII principle, this architecture benefit from Ref-counting "Manual"
 *	ViewModel instance, using it's TypedOuter, and is managed via a World Subsystem.
 *
 *	It follows the core architecture of Unreal Engine and ensure consistent data
 *	between Travels (as it's being rebuilt with the owning Actor).
 *
 *	This system is a work in-progress and has for end goal to remove some of the drawback affecting data persistency which comes
 *	with "Global" ViewModel usage.
 *	It's core benefit being that active ViewModel "presenting" can easily be swapped at Runtime based on the Actor it's owned by and
 *	"present" new data without running complex control flow for project specific use-case.
 *
 *	See Github link for more! : https://github.com/guyllaumedemers/UE-AVVM.

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
