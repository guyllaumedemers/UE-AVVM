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

#include "WorldPartition/WorldPartitionEditorPerProjectUserSettings.h"

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
	const auto OnSelectElement = [](const TWeakObjectPtr<const AActor>& OtherClusterElement)
	{
		return false;
	};

	const double NewElement_DistSquared = FVector::DistSquared(GetActorLocation(), NewTarget->GetActorLocation());
	const auto IsElementLessThan = [this, &NewElement_DistSquared](const TWeakObjectPtr<const AActor>& OtherClusterElement)
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

	const auto OnPushNewElement = [NewElement = TWeakObjectPtr(NewTarget)]()
	{
		return NewElement;
	};

	ClusterElements.Push(OnSelectElement, IsElementLessThan, OnPushNewElement);
	UpdateBeaconTransform();
}

bool AAVVMBeaconClusterActor::RemoveFromCluster(const AActor* NewTarget)
{
	return true;
}

const AActor* AAVVMBeaconClusterActor::GetClosestClusterElement(const AActor* OtherActor) const
{
	return nullptr;
}

void AAVVMBeaconClusterActor::UpdateBeaconTransform()
{
	FVector OutAverageLocation{FVector::ZeroVector};
	int32 OutCount{0};

	// @gdemers Prob the only valid use of capturing by ref when defining a capture clause...
	ClusterElements.SumV2([&OutAverageLocation, &OutCount](const TWeakObjectPtr<const AActor>& ClusterElement)
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
	const auto OnSelectElement = [Target = TWeakObjectPtr(PartitionActor), &MaxBeaconRadiusSquared](const TWeakObjectPtr<AActor>& BeaconActor)
	{
		if (!BeaconActor.IsValid() || !Target.IsValid())
		{
			return false;
		}

		const double DistSquared = FVector::DistSquared(BeaconActor->GetActorLocation(), Target->GetActorLocation());
		return FMath::IsWithin(DistSquared, 0.1, MaxBeaconRadiusSquared);
	};

	double MinDistSquared{DBL_MAX};
	const auto IsElementLessThan = [Target = TWeakObjectPtr(PartitionActor), &MinDistSquared](const TWeakObjectPtr<AActor>& BeaconActor)
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

	const auto OnPushNewElement = [this, NewWorld = TWeakObjectPtr(World), NewElement = TWeakObjectPtr(PartitionActor)]()
	{
		AAVVMBeaconClusterActor* Result = nullptr;
		if (!NewElement.IsValid() || !NewWorld.IsValid())
		{
			return Result;
		}

		Result = Factory(NewWorld.Get(), NewElement->GetTransform(), FActorSpawnParameters{});
		Result->SetClusterId(FMath::Rand());
		return Result;
	};

	auto* TreeNode = Clusters.Push(OnSelectElement, IsElementLessThan, OnPushNewElement);
	if (!ensureAlwaysMsgf(TreeNode != nullptr, TEXT("Invalid MemBlock referenced.")))
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}

	auto* Beacon = Cast<AAVVMBeaconClusterActor>(TreeNode->Value.Get());
	if (IsValid(Beacon))
	{
		Beacon->AddToCluster(PartitionActor);
		return FAVVMClusterObjectHandle{Beacon->GetClusterId(), Beacon};
	}
	else
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}
}

bool FAVVMClusterSystem::PopPartition(const FAVVMClusterObjectHandle& Handle)
{
	return true;
}

const AActor* FAVVMClusterSystem::GetClosestOverlappingObject(const FAVVMClusterObjectHandle& Handle,
                                                              const AActor* OtherActor) const
{
	const auto* Beacon = Cast<AAVVMBeaconClusterActor>(Handle.OwnedActor.Get());
	return ensureAlwaysMsgf(IsValid(Beacon), TEXT("Invalid Beacon referenced.")) ? Beacon->GetClosestClusterElement(OtherActor) : nullptr;
}
