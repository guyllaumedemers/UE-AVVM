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
#include "Effect/GameplayEffect_ProjectileTemplate.h"

#include "NonReplicatedProjectileActor.h"
#include "Kismet/GameplayStatics.h"

void FProjectileParams::Init(ANonReplicatedProjectileActor* Projectile,
                             const TArray<AActor*>& IgnoredActors) const &
{
	if (!IsValid(Projectile))
	{
		return;
	}

	const FTransform& ProjectileWorldTransform = Projectile->GetTransform();
	const FVector NormalizedDirection = ProjectileWorldTransform.Rotator().Vector();

	auto Params = FPredictProjectilePathParams(Radius, ProjectileWorldTransform.GetLocation(), (NormalizedDirection * Speed), MaxSimTime, ECollisionChannel::ECC_Visibility);
	Params.ActorsToIgnore.Append(IgnoredActors);

	FPredictProjectilePathResult OutResult;
	const bool bIsBlockingHit = UGameplayStatics::PredictProjectilePath(Projectile, Params, OutResult);

	Projectile->ProjectileTemplate = TInstancedStruct<FProjectileParams>::Make(*this);
	Projectile->bDoesPredictBlockingHit = bIsBlockingHit;
	Projectile->PredictedPathResult = OutResult;

	// TODO @gdemers we may want to not tick and kill right away if theres no blocking hit.
	Projectile->SetActorTickEnabled(true);
}

UScriptStruct* TBaseStructure<FProjectileParams>::Get()
{
	return FProjectileParams::StaticStruct();
}

void FExplosionParams::Init(ANonReplicatedProjectileActor* Projectile) const &
{
	if (IsValid(Projectile))
	{
		Projectile->ExplosionTemplate = TInstancedStruct<FExplosionParams>::Make(*this);
	}
}

UScriptStruct* TBaseStructure<FExplosionParams>::Get()
{
	return FExplosionParams::StaticStruct();
}
