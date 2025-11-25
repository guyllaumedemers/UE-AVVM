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

#include "Components/ActorComponent.h"

#include "AVVMReplicatedTraceComponent.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMReplicatedTrace is a context struct that encapsulate trace information.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMReplicatedTrace
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *	
 *	UAVVMReplicatedTraceComponent is a general purpose component for tracing in world based on requirements
 *	defined by the owning Outer.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMReplicatedTraceComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UAVVMReplicatedTraceComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	// @gdemers trace generated onTick
	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TArray<FAVVMReplicatedTrace> TickTraces;

	// @gdemers trace generated via input
	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TArray<FAVVMReplicatedTrace> BurstTraces;
};
