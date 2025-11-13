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

#include "AVVMNotificationSubsystem.h" // @gdemers fwd not supported, required due to UFUNCTION() / unreal reflection.
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"

#include "ProjectileManagerSubsystem.generated.h"

class ANonReplicatedProjectileActor;
class APlayerController;
class APlayerState;
struct FProjectileParams;

/**
 *	Class description:
 *
 *	FProjectileContextArgs is a context struct that encapsulate data required for projectile creation.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FProjectileContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	const UClass* ProjectileClass = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	TInstancedStruct<FProjectileParams> ProjectileParams;

	UPROPERTY(Transient, BlueprintReadWrite)
	FTransform AimTransform = FTransform::Identity;
};

/**
 *	Class description:
 *
 *	UProjectileManagerSubsystem is a subsystem tracking projectile location and notify
 *	external sources of PassBy events or react to actor pooling request.
 *
 *	External should bind to events on this system to react to projectile events
 *	that require conditional checks on Tick events.
 */
UCLASS()
class WEAPONSAMPLE_API UProjectileManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintCallable)
	static void Static_Register(const UWorld* World,
	                            ANonReplicatedProjectileActor* Projectile);

	UFUNCTION(BlueprintCallable)
	static void Static_Unregister(const UWorld* World,
	                              ANonReplicatedProjectileActor* Projectile);

	UFUNCTION(BlueprintCallable)
	static void Static_CreateProjectile(const UWorld* World,
	                                    const FProjectileContextArgs& ContextArgs);

protected:
	static UProjectileManagerSubsystem* Get(const UWorld* World);
	void Register(ANonReplicatedProjectileActor* Projectile);
	void Unregister(ANonReplicatedProjectileActor* Projectile);
	void CreateProjectile(const FProjectileContextArgs& ContextArgs) const;

	UFUNCTION(CallInEditor)
	void OnPlayerStateAddedOrRemoved(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload);
	
	void OnPlayerStateRemoved(const APlayerState* PlayerState);
	void OnPlayerStateAdded(const APlayerState* PlayerState);
	void OnProjectileShutdownRequested(ANonReplicatedProjectileActor* Projectile);

	// @gdemers factory method to support pooling or other instancing system specific to your project.
	virtual ANonReplicatedProjectileActor* Factory(const UClass* ProjectileClass, const FTransform& AimTransform) const;
	// @gdemers shutdown method to support pooling or other instancing system specific to your project.
	virtual void Shutdown(ANonReplicatedProjectileActor* Projectile);

	void HandleClientPassByBullets();
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const APlayerController> ClientPlayerController = nullptr;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<ANonReplicatedProjectileActor>> Projectiles;
};
