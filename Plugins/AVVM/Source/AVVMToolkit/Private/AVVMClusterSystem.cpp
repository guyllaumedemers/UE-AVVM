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
	return ClusterElements.IsEmpty();
}

void AAVVMBeaconClusterActor::AddToCluster(const AActor* NewTarget)
{
	if (!IsValid(NewTarget))
	{
		return;
	}
	
	// @gdemers we never want to select during expansion of a cluster until proven otherwise.
	const auto Select = [](const TWeakObjectPtr<const AActor>& OtherClusterElement)
	{
		return false;
	};

	const double NewElement_DistSquared = FVector::DistSquared(GetActorLocation(), NewTarget->GetActorLocation());
	const auto GoLeft = [this, &NewElement_DistSquared](const TWeakObjectPtr<const AActor>& OtherClusterElement)
	{
		if (!OtherClusterElement.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(GetActorLocation(), OtherClusterElement->GetActorLocation());
		if (NewElement_DistSquared < DistSquared)
		{
			return true;
		}
		else
		{
			return false;
		}
	};
	
	auto* TreeNode = ClusterElements.Search(Select, GoLeft);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return;
	}
	
	TreeNode->Value = NewTarget;
	UpdateBeaconTransform();
}

bool AAVVMBeaconClusterActor::RemoveFromCluster(const AActor* NewTarget)
{
	if (!IsValid(NewTarget))
	{
		return false;
	}

	const auto Select = [Target = TWeakObjectPtr(NewTarget)](const TWeakObjectPtr<const AActor>& OtherClusterElement)
	{
		return Target.IsValid() && OtherClusterElement.IsValid() && (OtherClusterElement == Target);
	};

	const double NewElement_DistSquared = FVector::DistSquared(GetActorLocation(), NewTarget->GetActorLocation());
	const auto GoLeft = [this, &NewElement_DistSquared](const TWeakObjectPtr<const AActor>& OtherClusterElement)
	{
		if (!OtherClusterElement.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(GetActorLocation(), OtherClusterElement->GetActorLocation());
		if (NewElement_DistSquared < DistSquared)
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	auto* TreeNode = ClusterElements.Search(Select, GoLeft);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return false;
	}
	else
	{
		// TODO @gdemers We have to rebuild the graph here.
		TreeNode->Value.Reset();

		UpdateBeaconTransform();
		return true;
	}
}

void AAVVMBeaconClusterActor::UpdateBeaconTransform()
{
	FVector OutAverageLocation{FVector::ZeroVector};
	int32 OutCount{0};

	// @gdemers Prob the only valid use of capturing by ref when defining a capture clause...
	ClusterElements.Sum([&OutAverageLocation, &OutCount](const TWeakObjectPtr<const AActor>& ClusterElement)
	{
		if (ClusterElement.IsValid())
		{
			OutAverageLocation += ClusterElement->GetActorLocation();
			++OutCount;
		}
	});

	OutAverageLocation = (OutAverageLocation / ((OutCount > 0) ? OutCount : 1));
	SetActorLocation(OutAverageLocation);
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
	const auto Select = [Target = TWeakObjectPtr(PartitionActor), &MaxBeaconRadiusSquared](const TWeakObjectPtr<AActor>& BeaconActor)
	{
		if (!BeaconActor.IsValid() || !Target.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(BeaconActor->GetActorLocation(), Target->GetActorLocation());
		return FMath::IsWithin(DistSquared, 0.1, MaxBeaconRadiusSquared);
	};

	double MinDistSquared{DBL_MAX};
	const auto GoLeft = [Target = TWeakObjectPtr(PartitionActor), &MinDistSquared](const TWeakObjectPtr<AActor>& BeaconActor)
	{
		if (!BeaconActor.IsValid() || !Target.IsValid())
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

	auto* TreeNode = Clusters.Search(Select, GoLeft);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}

	auto* Beacon = Cast<AAVVMBeaconClusterActor>(TreeNode->Value.Get());
	if (!IsValid(Beacon))
	{
		Beacon = Factory(World, PartitionActor->GetTransform(), FActorSpawnParameters{});
		Beacon->SetClusterId(FMath::Rand());
		TreeNode->Value = Beacon;
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

	const auto Select = [SearchClusterId = Handle.ClusterId](const TWeakObjectPtr<AActor>& BeaconActor)
	{
		const auto* Beacon = Cast<AAVVMBeaconClusterActor>(BeaconActor.Get());
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
	const auto GoLeft = [Target = TWeakObjectPtr(PartitionActor), &MinDistSquared](const TWeakObjectPtr<AActor>& BeaconActor)
	{
		if (!BeaconActor.IsValid() || !Target.IsValid())
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

	auto* TreeNode = Clusters.Search(Select, GoLeft);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return false;
	}
	else
	{
		// TODO @gdemers We have to rebuild the graph here.
		auto* Beacon = Cast<AAVVMBeaconClusterActor>(TreeNode->Value.Get());
		if (IsValid(Beacon))
		{
			Beacon->RemoveFromCluster(PartitionActor);
		}

		return true;
	}
}
