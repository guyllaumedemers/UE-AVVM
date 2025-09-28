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

#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"

#include "NonReplicatedProjectileActor.generated.h"

class ANonReplicatedProjectileActor;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnProjectilePoolingRequestDelegate, ANonReplicatedProjectileActor* Projectile);

/**
 *	Class description:
 *
 *	FProjectileParams is a context struct that encapsulate properties specific to a projectile
 *	and offer user extensibility through usage of TInstancedStruct<T>.
 */
USTRUCT(BlueprintType)
struct FProjectileParams
{
	GENERATED_BODY()

	virtual ~FProjectileParams() = default;
	virtual void Init(ANonReplicatedProjectileActor* Projectile) const;
	virtual bool DoesExplode() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Speed = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Mass = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Lifetime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<AActor> ExplosionActorClass = nullptr;
};

/**
 *	Class description:
 *
 *	ANonReplicatedProjectileActor is a non-replicated actor that simulate projectile updates
 *	on both server-client.
 */
UCLASS(BlueprintType, Blueprintable)
class WEAPONSAMPLE_API ANonReplicatedProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	ANonReplicatedProjectileActor(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	FOnProjectilePoolingRequestDelegate OnProjectilePoolingRequest;

protected:
	void ApplyForce(const FVector& DeltaVelocity);

	struct FProjectileTraceArgs
	{
		const UWorld* World = nullptr;
		FVector TraceStart = FVector::ZeroVector;
		FVector TraceEnd = FVector::ZeroVector;
		FColor TraceColor = FColor::Red;
		bool bDoesTraceHavePersistentLines = false;
		float TraceLifeTime = 0.f;
		FCollisionQueryParams CollisionQuery = FCollisionQueryParams::DefaultQueryParam;
		ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldStatic;
	};

	// @gdemers override trace behaviour for custom projectile type.
	virtual void TraceDebug(const FProjectileTraceArgs& TraceArgs);
	virtual bool Trace(const FProjectileTraceArgs& TraceArgs, FHitResult& OutHitResult);
	virtual const FCollisionQueryParams& GetCollisionParams() const;
	virtual const ECollisionChannel GetCollisionChannel() const;

	void HandleHit(const FHitResult& HitResult);
	bool HasExceededLifetime() const;
	void UpdateLifetime(const float DeltaTime);
	void Kill();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnProjectileExplosion(const TSoftClassPtr<AActor>& ExplosionClass,
	                                        const FVector& ImpactPoint,
	                                        const FVector& ImpactNormal);

	// @gdemers may require override interfacing with a pooling system of sort.
	virtual void HandleProjectileExplosion(const TSoftClassPtr<AActor>& ExplosionClass,
	                                       const FVector& ImpactPoint,
	                                       const FVector& ImpactNormal) PURE_VIRTUAL(HandleProjectileExplosion, return;);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TInstancedStruct<FProjectileParams> ProjectileParams;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector RuntimeVelocity = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly)
	float RuntimeMass = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	float RuntimeLifetime = 0.f;

	friend struct FProjectileParams;
};

/**
 *	Class description:
 *
 *	UProjectileFunctionLibrary is a utility function library enforcing data encapsulation with Projectile class.
 */
UCLASS()
class WEAPONSAMPLE_API UProjectileFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FVector GetDeltaVelocity(const FVector& Velocity,
	                                const float Mass,
	                                const float DeltaTime);

	UFUNCTION(BlueprintCallable)
	static bool CheckDistance(const ANonReplicatedProjectileActor* Projectile,
	                          const AActor* Target,
	                          const float SquaredDistanceThreshold);

	UFUNCTION(BlueprintCallable)
	static bool IsLocallyOwned(const ANonReplicatedProjectileActor* Projectile);

	UFUNCTION(BlueprintCallable)
	static void HandleProjectileFx(const TInstancedStruct<FProjectileParams>& Params,
	                               const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable)
	static void ApplyDamage(AActor* Instigator,
	                        const TInstancedStruct<FProjectileParams>& Params,
	                        AActor* Target);
};
