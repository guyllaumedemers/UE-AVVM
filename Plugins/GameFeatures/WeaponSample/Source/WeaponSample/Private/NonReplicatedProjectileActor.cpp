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
#include "NonReplicatedProjectileActor.h"

#include "ProjectileManagerSubsystem.h"
#include "TriggeringActor.h"
#include "WeaponDebuggerSettings.h"
#include "WeaponSample.h"
#include "Data/ProjectileDefinitionDataAsset.h"
#include "Engine/DamageEvents.h"

ANonReplicatedProjectileActor::ANonReplicatedProjectileActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickBatching = true;
	bReplicates = false;
}

void ANonReplicatedProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	const auto* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	auto* ProjectileManagerSubsystem = UProjectileManagerSubsystem::GetSubsystem(World);
	if (IsValid(ProjectileManagerSubsystem))
	{
		ProjectileManagerSubsystem->Register(this);
	}
}

void ANonReplicatedProjectileActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	auto* ProjectileManagerSubsystem = UProjectileManagerSubsystem::GetSubsystem(World);
	if (IsValid(ProjectileManagerSubsystem))
	{
		ProjectileManagerSubsystem->Unregister(this);
	}
}

void ANonReplicatedProjectileActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bHasExceededLifetime = HasExceededLifetime();
	if (bHasExceededLifetime)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const FTransform& ProjectileWorldTransform = GetTransform();
	const FVector DeltaVelocity = UProjectileFunctionLibrary::GetDeltaVelocity(RuntimeVelocity, RuntimeMass, DeltaSeconds);

	FProjectileTraceArgs TraceArgs;
	TraceArgs.World = World;
	TraceArgs.TraceStart = ProjectileWorldTransform.GetLocation();
	TraceArgs.TraceEnd = TraceArgs.TraceStart + DeltaVelocity;
	TraceArgs.TraceColor = FColor::MakeRandomColor();
	TraceArgs.bDoesTraceHavePersistentLines = UWeaponDebuggerSettings::DoesDebugTraceShowPersistentLine();
	TraceArgs.TraceLifeTime = UWeaponDebuggerSettings::GetDebugTraceLifetime();
	TraceArgs.CollisionChannel = GetCollisionChannel();
	TraceArgs.CollisionQuery = GetCollisionParams();

	// @gdemers update actor location/direction.
	const FRotator NewDirection = DeltaVelocity.ToOrientationRotator();
	SetActorLocationAndRotation(TraceArgs.TraceEnd, NewDirection);

	// @gdemers update velocity based on force applied which is on the up-vector only.
	ApplyForce(DeltaVelocity * FVector::UpVector);

#if !UE_BUILD_SHIPPING
	if (FWeaponSampleModule::GetCVarWeaponDebugTrace()->GetBool())
	{
		TraceDebug(TraceArgs);
	}
#endif

	auto OutHit = FHitResult();
	const bool bHasHit = Trace(TraceArgs, OutHit);
	if (bHasHit)
	{
		HandleHit(OutHit);
		Kill();
	}
	else
	{
		UpdateLifetime(DeltaSeconds);
	}
}

void ANonReplicatedProjectileActor::ApplyForce(const FVector& DeltaVelocity)
{
	RuntimeVelocity += DeltaVelocity;
}

void ANonReplicatedProjectileActor::TraceDebug(const FProjectileTraceArgs& TraceArgs)
{
	DrawDebugLine(TraceArgs.World,
	              TraceArgs.TraceStart,
	              TraceArgs.TraceEnd,
	              TraceArgs.TraceColor,
	              TraceArgs.bDoesTraceHavePersistentLines,
	              TraceArgs.TraceLifeTime);
}

bool ANonReplicatedProjectileActor::Trace(const FProjectileTraceArgs& TraceArgs, FHitResult& OutHitResult)
{
	const UWorld* World = TraceArgs.World;
	if (IsValid(World))
	{
		return World->LineTraceSingleByChannel(OutHitResult,
		                                       TraceArgs.TraceStart,
		                                       TraceArgs.TraceEnd,
		                                       TraceArgs.CollisionChannel,
		                                       TraceArgs.CollisionQuery);
	}
	else
	{
		return false;
	}
}

const FCollisionQueryParams& ANonReplicatedProjectileActor::GetCollisionParams() const
{
	return FCollisionQueryParams::DefaultQueryParam;
}

const ECollisionChannel ANonReplicatedProjectileActor::GetCollisionChannel() const
{
	return ECollisionChannel::ECC_Pawn;
}

void ANonReplicatedProjectileActor::HandleHit(const FHitResult& HitResult)
{
	if (HasAuthority())
	{
		const auto* Params = ExplosionTemplate.GetPtr<FExplosionParams>();
		if (Params != nullptr)
		{
			// @gdemers spawn simulated explosion actor.
			UProjectileFunctionLibrary::HandleProjectileExplosion(Params->ExplosionClass,
			                                                      HitResult.ImpactPoint,
			                                                      HitResult.ImpactNormal);
		}
		else
		{
			// @gdemers apply replicated damage to target.
			UProjectileFunctionLibrary::ApplyDamage(this, Template, HitResult.GetActor());
		}
	}
	else
	{
		// @gdemers spawn local vfx and sounds.
		UProjectileFunctionLibrary::HandleProjectileFx(Template, HitResult);
	}
}

bool ANonReplicatedProjectileActor::HasExceededLifetime() const
{
	return RuntimeLifetime <= 0.f;
}

void ANonReplicatedProjectileActor::UpdateLifetime(const float DeltaTime)
{
	RuntimeLifetime -= DeltaTime;

	if (HasExceededLifetime())
	{
		Kill();
	}
}

void ANonReplicatedProjectileActor::Kill()
{
	OnProjectileShutdown.Broadcast(this);
	RuntimeLifetime = 0.f;
}

FVector UProjectileFunctionLibrary::GetDeltaVelocity(const FVector& Velocity,
                                                     const float Mass,
                                                     const float DeltaTime)
{
	check(FMath::IsNearlyZero(Mass));

	static constexpr float GravityFactor = 9800.f;
	const float GravityAcceleration = (-GravityFactor / Mass);

	// @gdemers x-component is forward.
	// @gdemers y-component is right.
	// @gdemers z-component is up.
	const FVector YX = Velocity.Projection();
	const FVector XZ = FVector(Velocity.Z, Velocity.X, Velocity.Y).Projection();
	const FVector ZX = FVector(Velocity.X, Velocity.Z, -Velocity.Y).Projection();
	const float MagYX = YX.Size();
	const float MagXZ = XZ.Size();
	const float MagZX = ZX.Size();

	// @gdemers calculate displacement for each component axis. to do this, we have to project in 2d space.
	const float DeltaY = ((MagYX * YX.CosineAngle2D(FVector::RightVector)) * DeltaTime);
	const float DeltaX = ((MagXZ * XZ.CosineAngle2D(FVector::RightVector)) * DeltaTime);
	const float DeltaZ = ((MagZX * ZX.CosineAngle2D(FVector::RightVector)) * DeltaTime) - (0.5f * GravityAcceleration * DeltaTime * DeltaTime);

	// @gdemers output delta calculation with components in their correct slots.
	return FVector(DeltaX, DeltaY, DeltaZ);
}

bool UProjectileFunctionLibrary::CheckDistance(const ANonReplicatedProjectileActor* Projectile,
                                               const AActor* Target,
                                               const float SquaredDistanceThreshold)
{
	if (!IsValid(Projectile) || !IsValid(Target))
	{
		return false;
	}

	return (Projectile->GetSquaredDistanceTo(Target) <= SquaredDistanceThreshold);
}

bool UProjectileFunctionLibrary::IsLocallyOwned(const ANonReplicatedProjectileActor* Projectile)
{
	if (IsValid(Projectile))
	{
		const auto* PC = Projectile->GetTypedOuter<APlayerController>();
		return IsValid(PC) ? PC->IsLocalPlayerController() : false;
	}

	return false;
}

void UProjectileFunctionLibrary::HandleProjectileFx(const TInstancedStruct<FProjectileParams>& Params,
                                                    const FHitResult& HitResult)
{
	// TODO @gdemers handle pooling and vfx
}

void UProjectileFunctionLibrary::HandleProjectileExplosion(const TSoftClassPtr<ANonReplicatedExplosionActor>& ExplosionClass,
                                                           const FVector& ImpactPoint,
                                                           const FVector& ImpactNormal)
{
	// TODO @gdemers query a subsystem handling the explosion
}

void UProjectileFunctionLibrary::ApplyDamage(AActor* Instigator,
                                             const TInstancedStruct<FProjectileParams>& Params,
                                             AActor* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	// TODO @gdemers generate data based on stats system and other modifier types.
	float Damage = 0.f;
	FDamageEvent DamageEvent;
	AController* Controller = IsValid(Instigator) ? Instigator->GetTypedOuter<AController>() : nullptr;
	Target->TakeDamage(Damage, DamageEvent, Controller, Instigator);
}
