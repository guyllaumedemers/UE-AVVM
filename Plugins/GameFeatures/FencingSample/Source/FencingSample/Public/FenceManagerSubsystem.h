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

#include "Subsystems/WorldSubsystem.h"

#include "FenceManagerSubsystem.generated.h"

class UActorFenceComponent;

/**
 *	Class description:
 *
 *	UFenceManagerSubsystem is a Client-ONLY subsystem that captures UActorFenceComponent and notify external systems of status update.
 */
UCLASS()
class FENCINGSAMPLE_API UFenceManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllFencesRemoved);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFencesCountModified);

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static void Static_UnregisterFence(const UObject* WorldContextObject,
	                                   const UActorFenceComponent* NewActorFenceComponent);

	UFUNCTION(BlueprintCallable, meta=(HideSelfPin, DefaultToSelf="WorldContextObject"))
	static void Static_RegisterFence(const UObject* WorldContextObject,
	                                 const UActorFenceComponent* NewActorFenceComponent);

	UPROPERTY(BlueprintAssignable)
	FOnAllFencesRemoved OnAllFencesRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnFencesCountModified OnFencesCountModified;

protected:
	static UFenceManagerSubsystem* Get(const UObject* WorldContextObject);
	void Raise(const UActorFenceComponent* NewActorFenceComponent);
	void Lower(const UActorFenceComponent* NewActorFenceComponent);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<const UActorFenceComponent>> Fences;

	friend class UFenceCheatExtension;
};
