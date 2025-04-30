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
#include "AVVM.h"

#include "UObject/Object.h"

#include "AVVMSampleFactoryImpl.generated.h"

/**
 *	Class description:
 *
 *	UAVVMSampleFactoryImpl. provide implementation details about the UObject class to instance
 *	based on received flags.
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMSampleFactoryImpl : public UObject,
                                                     public IAVVMResolverFactoryImpl
{
	GENERATED_BODY()

public:
	virtual TScriptInterface<IAVVMResolverExecutioner> Factory(const EAVVMObserverResolverFlag ResolverFlag) const override;
};

/**
 *	Class description:
 *
 *	UAVVMSampleActorNameExecutioner. provide an implementation on how filtering should be perform using the Actor Name. Impplying we care about the unique
 *	instance of this Actor Class.
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMSampleActorNameExecutioner : public UObject,
                                                              public IAVVMResolverExecutioner
{
	GENERATED_BODY()

public:
	virtual TArray<TScriptInterface<IAVVMObserver>> Filter(const FString& MatchRequirement,
	                                                       const TArray<TScriptInterface<IAVVMObserver>>& Observers) const override;
};

/**
 *	Class description:
 *
 *	UAVVMSampleActorClassNameExecutioner. provide an implementation on how filtering should be perform using the Actor Class Name. Impplying we care about
 *	this Actor Class and not the instance.
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMSampleActorClassNameExecutioner : public UObject,
                                                                   public IAVVMResolverExecutioner
{
	GENERATED_BODY()

public:
	virtual TArray<TScriptInterface<IAVVMObserver>> Filter(const FString& MatchRequirement,
	                                                       const TArray<TScriptInterface<IAVVMObserver>>& Observers) const override;
};
