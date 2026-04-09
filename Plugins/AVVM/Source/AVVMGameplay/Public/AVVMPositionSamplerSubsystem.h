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

#include "Math/GenericOctree.h"
#include "Subsystems/WorldSubsystem.h"

#include "AVVMPositionSamplerSubsystem.generated.h"

class AGameStateBase;

/**
 *	Class description:
 *	
 *	UAVVMPositionSamplerSubsystem is a tickable subsystem that capture actor position samples on the authoritative side (running prediction).
 *	Note : Currently, it focuses solely on playable character (and/or AI).
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMPositionSamplerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;

	static FBoxCenterAndExtent Static_GetSampleExtent(const UWorld* World, const AActor* Target, const double Timestamp);

protected:
	static UAVVMPositionSamplerSubsystem* Get(const UWorld* World);
	FBoxCenterAndExtent GetSampleExtent(const AActor* Target, const double Timestamp);
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AGameStateBase> GameStateBase = nullptr;

	struct FAVVMPositionSample
	{
		FBoxCenterAndExtent TransformExtent = FBoxCenterAndExtent();
		double SampleTimestamp = 0;
	};

	struct FAVVMPositionSampler
	{
		FAVVMPositionSampler();
		FBoxCenterAndExtent GetClosestSample(const double Timestamp) const;
		void Sample(const FAVVMPositionSample& NewSample);

	private:
		TArray<FAVVMPositionSample> Samples;
		int32 CurrSampleIndex = 0;
	};

	TMap<TWeakObjectPtr<const AActor>, FAVVMPositionSampler> AuthoritativePositionSamplers;
};
