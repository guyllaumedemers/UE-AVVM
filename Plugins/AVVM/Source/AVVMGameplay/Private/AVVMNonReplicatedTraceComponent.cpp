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
#include "AVVMNonReplicatedTraceComponent.h"

#include "AVVMTickScheduler.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UAVVMNonReplicatedTraceComponent::UAVVMNonReplicatedTraceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(false);
}

void UAVVMNonReplicatedTraceComponent::BeginPlay()
{
	Super::BeginPlay();

#if !WITH_SERVER_CODE || WITH_EDITOR
	auto* Character = GetTypedOuter<ACharacter>();
	if (IsValid(Character) && Character->IsLocallyControlled())
	{
		UAVVMTickScheduler::Static_Register(GetWorld(), this);
	}
#endif
}

void UAVVMNonReplicatedTraceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if !WITH_SERVER_CODE || WITH_EDITOR
	auto* Character = GetTypedOuter<ACharacter>();
	if (IsValid(Character) && Character->IsLocallyControlled())
	{
		UAVVMTickScheduler::Static_UnRegister(GetWorld(), this);
	}
#endif
}

void UAVVMNonReplicatedTraceComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/**
	 *	Always left-bottom, to top-right.
	 */
	static const FVector TraceDirections[26]
	{
			// front
			(FVector::DownVector + FVector::LeftVector + FVector::ForwardVector)/*0*/,
			(FVector::DownVector + FVector::ForwardVector)/*1*/,
			(FVector::DownVector + FVector::RightVector + FVector::ForwardVector)/*2*/,
			(FVector::LeftVector + FVector::ForwardVector)/*3*/,
			(FVector::ForwardVector)/*4*/,
			(FVector::RightVector + FVector::ForwardVector)/*5*/,
			(FVector::UpVector + FVector::LeftVector + FVector::ForwardVector)/*6*/,
			(FVector::UpVector + FVector::ForwardVector)/*7*/,
			(FVector::UpVector + FVector::RightVector + FVector::ForwardVector)/*8*/,
			// back
			(FVector::DownVector + FVector::LeftVector + FVector::BackwardVector)/*9*/,
			(FVector::DownVector + FVector::BackwardVector)/*10*/,
			(FVector::DownVector + FVector::RightVector + FVector::BackwardVector)/*11*/,
			(FVector::LeftVector + FVector::BackwardVector)/*12*/,
			(FVector::BackwardVector)/*13*/,
			(FVector::RightVector + FVector::BackwardVector)/*14*/,
			(FVector::UpVector + FVector::LeftVector + FVector::BackwardVector)/*15*/,
			(FVector::UpVector + FVector::BackwardVector)/*16*/,
			(FVector::UpVector + FVector::RightVector + FVector::BackwardVector)/*17*/,
			// sides
			(FVector::DownVector + FVector::LeftVector)/*18*/,
			(FVector::LeftVector)/*19*/,
			(FVector::UpVector + FVector::LeftVector)/*20*/,
			(FVector::UpVector)/*21*/,
			(FVector::UpVector + FVector::RightVector)/*22*/,
			(FVector::RightVector)/*23*/,
			(FVector::DownVector + FVector::RightVector)/*24*/,
			(FVector::DownVector)/*25*/,
	};

	const UWorld* World = GetWorld();
	if (!IsValid(World) || !OwningOuter.IsValid())
	{
		return;
	}

	const FTransform WorldTransform = OwningOuter->GetTransform();
	const FVector TraceStart = WorldTransform.GetLocation();
	const float CenterOffset_VoxelCell = ((OuterCapsuleHalfHeight * VoxelCellPadding) + SphereRadius);

	// 4, 13, 19, 21, 23, 25 should be unit length
	static const TSet<int32> IndexesOfUnitLength(TArrayView<const int32>{4, 13, 19, 21, 23, 25});

	TArray<FHitResult*> OutResults;
	for (int32 i = 0; i < VoxelGrid.Cells.Num(); ++i)
	{
		FAVVMVoxelCell& Cell = VoxelGrid.Cells[i];
		if (!Cell.bIsEnabled)
		{
			continue;
		}

		const FVector& PreBuiltTraceDirection = TraceDirections[i];
		// get evenly distributed position of our sphere trace center
		FVector ScaledDirection = PreBuiltTraceDirection.GetSafeNormal();
		if (!IndexesOfUnitLength.Contains(i))
		{
			ScaledDirection *= PreBuiltTraceDirection.Size();
		}

		const FVector TraceEnd = (TraceStart + (ScaledDirection * CenterOffset_VoxelCell));
		const bool bHasBlockingHit = World->SweepSingleByChannel(Cell.Result, TraceStart, TraceEnd, FQuat::Identity, CollisionChannel, FCollisionShape::MakeSphere(SphereRadius));
		if (bHasBlockingHit)
		{
			OutResults.Add(&Cell.Result);
		}

		// TODO Define what I want to do with the trace
	}
}

void UAVVMNonReplicatedTraceComponent::OnOuterCapsuleResized(const UCapsuleComponent* ModifiedCapsule)
{
	if (IsValid(ModifiedCapsule))
	{
		OuterCapsuleHalfHeight = ModifiedCapsule->GetScaledCapsuleHalfHeight();
	}
}
