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

#include "AVVMDoesSupportManualTicking.h"
#include "Components/ActorComponent.h"

#include "AVVMNonReplicatedTraceComponent.generated.h"

class UCapsuleComponent;

/**
 *	Class description:
 *	
 *	UAVVMNonReplicatedTraceComponent is a general purpose component for tracing in world based on requirements
 *	defined by the owning Outer.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMNonReplicatedTraceComponent : public UActorComponent,
                                                          public IAVVMDoesSupportManualTicking
{
	GENERATED_BODY()
	
public:
	UAVVMNonReplicatedTraceComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	/**
	 *	Class description:
	 *	
	 *	FAVVMVoxelCell is a context struct that represent an entry within a Voxel Grid.
	 */
	struct FAVVMVoxelCell
	{
		FHitResult Result = FHitResult();
		bool bIsEnabled = false;
	};
	
	/**
	 *	Class description:
	 *	
	 *	FAVVMVoxelGrid is a context struct that generate world trace surrounding
	 *	the type outer position in 3d world.
	 */
	struct FAVVMVoxelGrid
	{
		FAVVMVoxelGrid()
		{
			static constexpr int32 NumCells = 26;
			Cells.Reserve(NumCells);
		}

		TArray<FAVVMVoxelCell> Cells;
	};
	
	UFUNCTION()
	void OnOuterCapsuleResized(const UCapsuleComponent* ModifiedCapsule);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Designers")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_MAX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Designers", meta=(ClampMin="0", ClampMax="999"))
	float VoxelCellPadding = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Designers", meta=(ClampMin="0", ClampMax="999"))
	float SphereRadius = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	float OuterCapsuleHalfHeight = 0.f;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	FAVVMVoxelGrid VoxelGrid = FAVVMVoxelGrid();
};
