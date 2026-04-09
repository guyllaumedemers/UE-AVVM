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
#include "AVVMPositionSamplerSubsystem.h"

#include "AVVMCharacter.h"
#include "AVVMWorldSetting.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

bool UAVVMPositionSamplerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	const bool bIsGameWorld = IsValid(World) ? World->IsGameWorld() : false;
	if (!bIsGameWorld || (World->IsNetMode(NM_Standalone)/*NoPrediction*/ || World->IsNetMode(NM_Client)/*Non-Authoritative*/))
	{
		return false;
	}

	return true;
}

void UAVVMPositionSamplerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GameStateBase = UGameplayStatics::GetGameState(this);
}

void UAVVMPositionSamplerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	GameStateBase.Reset();
}

void UAVVMPositionSamplerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const AGameStateBase* NewGameStateBase = GameStateBase.Get();
	if (!IsValid(NewGameStateBase))
	{
		return;
	}

	for (TActorIterator<AAVVMCharacter> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		auto* Player = (*Iterator);
		if (!IsValid(Player))
		{
			continue;
		}

		const auto* CapsuleComponent = (*Iterator)->GetCapsuleComponent();
		if (!ensureAlwaysMsgf(IsValid(CapsuleComponent), TEXT("Collision Bounds are undefined on the server!")))
		{
			continue;
		}

		FAVVMPositionSample NewSample;
		NewSample.SampleTimestamp = NewGameStateBase->GetServerWorldTimeSeconds();
		NewSample.TransformExtent = FBoxCenterAndExtent(CapsuleComponent->Bounds);

		FAVVMPositionSampler& PositionSampler = AuthoritativePositionSamplers.FindOrAdd(Player);
		PositionSampler.Sample(NewSample);
	}
}

FBoxCenterAndExtent UAVVMPositionSamplerSubsystem::Static_GetSampleExtent(const UWorld* World,
                                                                          const AActor* Target,
                                                                          const double Timestamp)
{
	auto* Subsystem = UAVVMPositionSamplerSubsystem::Get(World);
	return IsValid(Subsystem) ? Subsystem->GetSampleExtent(Target, Timestamp) : FBoxCenterAndExtent();
}

UAVVMPositionSamplerSubsystem* UAVVMPositionSamplerSubsystem::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAVVMPositionSamplerSubsystem>(World);
}

FBoxCenterAndExtent UAVVMPositionSamplerSubsystem::GetSampleExtent(const AActor* Target, const double Timestamp)
{
	const FAVVMPositionSampler* SearchResult = AuthoritativePositionSamplers.Find(Target);
	if (SearchResult != nullptr)
	{
		return SearchResult->GetClosestSample(Timestamp);
	}
	else
	{
		return FBoxCenterAndExtent();
	}
}

UAVVMPositionSamplerSubsystem::FAVVMPositionSampler::FAVVMPositionSampler()
{
	static constexpr int32 SampleSize = 100;
	Samples.Init(FAVVMPositionSample(), SampleSize);
}

FBoxCenterAndExtent UAVVMPositionSamplerSubsystem::FAVVMPositionSampler::GetClosestSample(const double Timestamp) const
{
	const FAVVMPositionSample* ClosestLowerBound = nullptr;
	const FAVVMPositionSample* ClosestUpperBound = nullptr;

	for (int32 i = 0; i < Samples.Num(); ++i)
	{
		const FAVVMPositionSample& CurrSample = Samples[i];
		if ((CurrSample.SampleTimestamp <= Timestamp) && ((ClosestLowerBound == nullptr) || (ClosestLowerBound->SampleTimestamp < CurrSample.SampleTimestamp)))
		{
			ClosestLowerBound = &CurrSample;
		}

		if ((CurrSample.SampleTimestamp >= Timestamp) && ((ClosestUpperBound == nullptr) || (ClosestUpperBound->SampleTimestamp > CurrSample.SampleTimestamp)))
		{
			ClosestUpperBound = &CurrSample;
		}
	}

	FBoxCenterAndExtent Result;
	if (!ensureAlways(ClosestLowerBound != nullptr) || !ensureAlways(ClosestUpperBound != nullptr))
	{
		return Result;
	}

	const double NormalizedDelta = (Timestamp - ClosestLowerBound->SampleTimestamp) / (ClosestUpperBound->SampleTimestamp - ClosestLowerBound->SampleTimestamp);
	const FVector NewOrigin = ClosestLowerBound->TransformExtent.Center + ((ClosestUpperBound->TransformExtent.Center - ClosestLowerBound->TransformExtent.Center) * NormalizedDelta);
	Result = FBoxCenterAndExtent(NewOrigin, ClosestLowerBound->TransformExtent.Extent);

	return Result;
}

void UAVVMPositionSamplerSubsystem::FAVVMPositionSampler::Sample(const FAVVMPositionSample& NewSample)
{
	Samples[CurrSampleIndex] = NewSample;
	CurrSampleIndex = ((CurrSampleIndex + 1) % Samples.Num());
}
