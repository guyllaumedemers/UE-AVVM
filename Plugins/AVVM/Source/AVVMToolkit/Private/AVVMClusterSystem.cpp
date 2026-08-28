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

FAVVMClusterObjectHandle FAVVMClusterSystem::PushPartition(UWorld* World, const AActor* PartitionActor)
{
	if (!IsValid(PartitionActor))
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}

	// TODO @gdemers try converting this to consteval
	const double MaxBeaconRadiusSquared{GetMaximumBeaconRadius() * GetMaximumBeaconRadius()};
	const auto Select = [Target = TWeakObjectPtr(PartitionActor), &MaxBeaconRadiusSquared](const AActor* BeaconActor)
	{
		if (!IsValid(BeaconActor) || !Target.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(BeaconActor->GetActorLocation(), Target->GetActorLocation());
		return FMath::IsWithin(DistSquared, 0.1, MaxBeaconRadiusSquared);
	};

	double MinDistSquared{DBL_MAX};
	const auto GoLeft = [Target = TWeakObjectPtr(PartitionActor), &MinDistSquared](const AActor* BeaconActor)
	{
		if (!IsValid(BeaconActor) || !Target.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(BeaconActor->GetActorLocation(), Target->GetActorLocation());
		if (DistSquared < MinDistSquared)
		{
			MinDistSquared = DistSquared;
			return true;
		}
		else
		{
			return false;
		}
	};

	auto* TreeNode = BST_ClusterGraph.Search(Select, GoLeft);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}

	auto* Beacon = Cast<AAVVMBeaconClusterActor>(TreeNode->Entity.Get());
	if (!IsValid(Beacon))
	{
		Beacon = Factory(World, PartitionActor->GetTransform(), FActorSpawnParameters{});
		Beacon->SetClusterId(FMath::Rand());
		TreeNode->Entity = Beacon;
	}

	Beacon->AddToCluster(PartitionActor);
	return FAVVMClusterObjectHandle{Beacon->GetClusterId(), Beacon};
}

bool FAVVMClusterSystem::PopPartition(const FAVVMClusterObjectHandle& Handle)
{
	const auto* PartitionActor = Handle.OwnedActor.Get();
	if (!IsValid(PartitionActor))
	{
		return false;
	}

	const auto Select = [SearchClusterId = Handle.ClusterId](const AActor* BeaconActor)
	{
		const auto* Beacon = Cast<AAVVMBeaconClusterActor>(BeaconActor);
		if (!IsValid(Beacon))
		{
			return false;
		}
		else
		{
			return Beacon->GetClusterId() == SearchClusterId;
		}
	};

	double MinDistSquared{DBL_MAX};
	const auto GoLeft = [Target = TWeakObjectPtr(PartitionActor), &MinDistSquared](const AActor* BeaconActor)
	{
		if (!IsValid(BeaconActor) || !Target.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(BeaconActor->GetActorLocation(), Target->GetActorLocation());
		if (DistSquared < MinDistSquared)
		{
			MinDistSquared = DistSquared;
			return true;
		}
		else
		{
			return false;
		}
	};

	auto* TreeNode = BST_ClusterGraph.Search(Select, GoLeft);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return false;
	}
	else
	{
		// TODO @gdemers We have to rebuild the graph here.
		auto* Beacon = Cast<AAVVMBeaconClusterActor>(TreeNode->Entity.Get());
		if (IsValid(Beacon))
		{
			Beacon->RemoveFromCluster(PartitionActor);
		}

		return true;
	}
}
