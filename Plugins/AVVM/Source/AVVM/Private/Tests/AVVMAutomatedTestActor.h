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

#include "AVVMNotificationSubsystem.h"
#include "GameFramework/Actor.h"

#include "AVVMAutomatedTestActor.generated.h"

class UAVVMComponent;
class UAVVMPresenter;

/**
 *	Class description:
 *	
 *	FScopedCounterNotify is a scoped class to track amount of notification executed based on user expectation.
 */
struct FScopedCounterNotify
{
	explicit FScopedCounterNotify(const int32 NewCounter)
		: Counter(NewCounter), Default(NewCounter)
	{
	}

	void Reset()
	{
		Counter = Default;
	}

	int32 GetCount() const
	{
		return Counter;
	}

	void Minus()
	{
		--Counter;
	}

private:
	int32 Counter = INDEX_NONE;
	int32 Default = INDEX_NONE;
};

/**
 *	Class description:
 *
 *	AAVVMAutomatedTestActor is an Actor class to run behaviour during Automated Testing.
 */
UCLASS()
class AVVM_API AAVVMAutomatedTestActor : public AActor
{
	GENERATED_BODY()

public:
	AAVVMAutomatedTestActor(const FObjectInitializer& ObjectInitializer);
	
	void GetSetDelegate(FScopedCounterNotify* NewScopedCounterNotify,
	                    FAVVMOnChannelNotifiedSingleCastDelegate& OutDelegate);

	UFUNCTION(CallInEditor)
	void OnInvokeCallback(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMComponent> AVVMComponent = nullptr;

private:
	// @gdemers pure lazyness here.
	FScopedCounterNotify* ScopedCounterNotify;

	friend class UAVVMAutomatedTestUtils;
};

/**
 * 
 */
UCLASS()
class UAVVMAutomatedTestUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UAVVMPresenter* GetSetPresenter(const TSubclassOf<UAVVMPresenter>& PresenterClass,
	                                       AAVVMAutomatedTestActor* Actor);
};
