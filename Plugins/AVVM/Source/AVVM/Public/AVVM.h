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

#include "AVVM.generated.h"

enum class EAVVMObserverResolverFlag : uint8;
class IAVVMObserver;

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
 */

// ---------------------------------------------------------------------------------------------------------------------//
//									Required implementation in project													//
// ---------------------------------------------------------------------------------------------------------------------//

/**
 *	Class description:
 *
 *	UAVVMResolverExecutioner. define an interface to be implemented by a UObject class. Like a predicate, we expect the implementation
 *	to define hard constraint for equality.
 *
 *	Example : UActorNameExecutioner -> Search through the observer collection for a match based on the Actor::GetFName()::ToString().Equals(MatchRequirement)
 *	vs. UActorClassExecutioner -> Search through the observer collection for a match based on the Actor::GetClass()::ToString().Equals(MatchRequirement)
 */
UINTERFACE(BlueprintType)
class AVVM_API UAVVMResolverExecutioner : public UInterface
{
	GENERATED_BODY()
};

class AVVM_API IAVVMResolverExecutioner
{
	GENERATED_BODY()

public:
	// @gdemers filter the list of observers based on class implementation.
	virtual TArray<TScriptInterface<IAVVMObserver>> Filter(const FString& MatchRequirement,
	                                                       const TArray<TScriptInterface<IAVVMObserver>>& Observers) const PURE_VIRTUAL(Filter, return {};);
};

/**
 *	Class description:
 *
 *	UAVVMResolverFactoryImpl. project specific interface that define what Executioner UObject should be created based on the received
 *	flag.
 */
UINTERFACE(BlueprintType)
class AVVM_API UAVVMResolverFactoryImpl : public UInterface
{
	GENERATED_BODY()
};

class AVVM_API IAVVMResolverFactoryImpl
{
	GENERATED_BODY()

public:
	// @gdemers factory method to override with project impl.
	virtual TScriptInterface<IAVVMResolverExecutioner> Factory(const EAVVMObserverResolverFlag ResolverFlag) const PURE_VIRTUAL(Factory, return nullptr;);
};
