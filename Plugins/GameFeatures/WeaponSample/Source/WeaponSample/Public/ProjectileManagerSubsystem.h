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

#include "ProjectileManagerSubsystem.generated.h"

class ANonReplicatedProjectileActor;
class APlayerController;
class APlayerState;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnProjectilePassByDelegate, const FVector& Location)

/**
 *	Class description:
 *
 *	UProjectileManagerSubsystem is a subsystem tracking projectile location and notify
 *	external sources of PassBy events or react to actor pooling request.
 *
 *	External should bind to events on this system to react to projectile events
 *	that require conditional checks on Tick events.
 */
UCLASS(ClassGroup=("Weapon"))
class WEAPONSAMPLE_API UProjectileManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void Register(ANonReplicatedProjectileActor* Projectile);
	
	UFUNCTION(BlueprintCallable)
	void Unregister(ANonReplicatedProjectileActor* Projectile);
	
	UFUNCTION(BlueprintCallable)
	static UProjectileManagerSubsystem* GetSubsystem(const UWorld* World);

	FOnProjectilePassByDelegate OnProjectilePassBy;

protected:
	void OnPlayerStateRemoved(APlayerState* PlayerState);
	void OnPlayerStateAdded(APlayerState* PlayerState);

	void OnProjectileRequestPooling(ANonReplicatedProjectileActor* Projectile);
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const APlayerController> ClientPlayerController = nullptr;
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ANonReplicatedProjectileActor>> Projectiles;
};
