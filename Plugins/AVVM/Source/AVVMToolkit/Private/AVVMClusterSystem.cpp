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
#include "AVVMClusterSystem.h"

FAVVMClusterObjectHandle FAVVMClusterObjectHandle::InvalidHandle{};

int32 AAVVMBeaconClusterActor::GetClusterId() const
{
	return ClusterId;
}

void AAVVMBeaconClusterActor::SetClusterId(const int32 NewClusterId)
{
	ClusterId = NewClusterId;
}

bool AAVVMBeaconClusterActor::IsClusterEmpty() const
{
	return Cluster.IsEmpty();
}

void AAVVMBeaconClusterActor::AddToCluster(const AActor* Target)
{
	Cluster.Add(Target);
	UpdateBeaconTransform();
}

void AAVVMBeaconClusterActor::RemoveFromCluster(const AActor* Target)
{
	const bool bDoesContain = Cluster.Contains(Target);
	if (bDoesContain)
	{
		Cluster.Remove(Target);
	}

	UpdateBeaconTransform();
}

void AAVVMBeaconClusterActor::UpdateBeaconTransform()
{
	FVector NewAverageLocation{FVector::ZeroVector};
	for (const auto& Actor : Cluster)
	{
		NewAverageLocation += (Actor.IsValid() ? Actor->GetActorLocation() : FVector::ZeroVector);
	}

	NewAverageLocation /= (Cluster.IsEmpty() ? 1.f : Cluster.Num());
	SetActorLocation(NewAverageLocation);
}

AAVVMBeaconClusterActor* FAVVMClusterSystem::Factory(UWorld* World,
                                                     const FTransform& SpawnTransform,
                                                     const FActorSpawnParameters& SpawnParams)
{
	if (IsValid(World))
	{
		return Cast<AAVVMBeaconClusterActor>(World->SpawnActor(GetBeaconActorClass(), &SpawnTransform, SpawnParams));
	}

	return nullptr;
}

TSubclassOf<AAVVMBeaconClusterActor> FAVVMClusterSystem::GetBeaconActorClass() const
{
	return AAVVMBeaconClusterActor::StaticClass();
}

double FAVVMClusterSystem::GetMaximumBeaconRadius() const
{
	// @gdemers abstract value of 10 meters;
	return 1000.f;
}

FAVVMClusterObjectHandle FAVVMClusterSystem::AppendOrCreateCluster(const AActor* Actor)
{
	if (!IsValid(Actor) || !ensureAlwaysMsgf(!MemoizationMap.Contains(Actor),
	                                         TEXT("Attempting to append duplicated actor.")))
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}

	double CurrClosestDistSquared = DBL_MAX;
	const double BeaconRangeSquared = (GetMaximumBeaconRadius() * GetMaximumBeaconRadius());

	const auto Predicate_GetClosestBeacon = [&](const AActor* Target)
	{
		if (!IsValid(Target) || !IsValid(Actor))
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(Target->GetActorLocation(), Actor->GetActorLocation());
		if ((DistSquared < CurrClosestDistSquared) && (DistSquared <= BeaconRangeSquared))
		{
			CurrClosestDistSquared = DistSquared;
			return true;
		}

		return false;
	};

	TArray<AActor*> OutOverlappingActors{};
	Actor->GetOverlappingActors(OutOverlappingActors, GetBeaconActorClass());

	const TArray<AActor*> OverlappingBeacons = OutOverlappingActors.FilterByPredicate(Predicate_GetClosestBeacon);
	AAVVMBeaconClusterActor* ClosestBeacon = nullptr;

	if (OverlappingBeacons.IsEmpty())
	{
		TArray<TObjectPtr<AAVVMBeaconClusterActor>> ClosestActors{};
		Beacons.GenerateValueArray(ClosestActors);

		ClosestActors = ClosestActors.FilterByPredicate(Predicate_GetClosestBeacon);
		if (ClosestActors.IsEmpty())
		{
			AAVVMBeaconClusterActor* NewBeacon = Factory(Actor->GetWorld(), Actor->GetActorTransform(), FActorSpawnParameters{});
			if (ensureAlwaysMsgf(IsValid(NewBeacon), TEXT("Failed to allocate memory for actor creation.")))
			{
				NewBeacon->SetClusterId(FMath::Rand());
				Beacons.Add(NewBeacon->GetClusterId(), NewBeacon);
				ClosestBeacon = NewBeacon;
			}
		}
		else
		{
			ClosestBeacon = Cast<AAVVMBeaconClusterActor>(ClosestActors.Top());
		}
	}
	else
	{
		ClosestBeacon = Cast<AAVVMBeaconClusterActor>(OverlappingBeacons.Top());
	}

	int32& OutClusterId = MemoizationMap.Add(Actor);
	if (ensureAlwaysMsgf(IsValid(ClosestBeacon), TEXT("Invalid Beacon Actor.")))
	{
		OutClusterId = ClosestBeacon->GetClusterId();
		ClosestBeacon->AddToCluster(Actor);
	}

	FAVVMClusterObjectHandle NewHandle{OutClusterId, Actor};
	return MoveTemp(NewHandle);
}

bool FAVVMClusterSystem::RemoveFromCluster(const FAVVMClusterObjectHandle& Handle)
{
	if (!ensureAlwaysMsgf(MemoizationMap.Contains(Handle.OwnedActor), TEXT("Trying to remove invalid Actor.")) ||
		!ensureAlwaysMsgf(Beacons.Contains(Handle.ClusterId), TEXT("Trying to remove invalid handle.")))
	{
		return false;
	}

	TObjectPtr<AAVVMBeaconClusterActor>& OutResult = Beacons[Handle.ClusterId];
	if (!ensureAlwaysMsgf(IsValid(OutResult), TEXT("Invalid Beacon Actor.")))
	{
		return false;
	}

	MemoizationMap.Remove(Handle.OwnedActor);
	OutResult->RemoveFromCluster(Handle.OwnedActor.Get());
	if (OutResult->IsClusterEmpty())
	{
		Beacons.Remove(Handle.ClusterId);
	}

	return true;
}
