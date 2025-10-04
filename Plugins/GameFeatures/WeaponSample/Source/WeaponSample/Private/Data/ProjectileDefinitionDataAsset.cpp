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
#include "Data/ProjectileDefinitionDataAsset.h"

#include "NonReplicatedProjectileActor.h"

void FProjectileParams::Init(ANonReplicatedProjectileActor* Projectile) const
{
	if (!IsValid(Projectile))
	{
		return;
	}

	// @gdemers initialize actor velocity based on direction assigned during construction.
	const FTransform& ProjectileWorldTransform = Projectile->GetTransform();
	const FVector NormalizedDirection = ProjectileWorldTransform.Rotator().Vector();

	// TODO @gdemers Update this later to reduce allocation made by TInstancedStruct.
	Projectile->Template = TInstancedStruct<FProjectileParams>::Make(*this);
	Projectile->RuntimeVelocity = (NormalizedDirection * Speed);
	Projectile->RuntimeMass = Mass;
}

void FExplosionParams::Init(ANonReplicatedProjectileActor* Projectile) const
{
	if (IsValid(Projectile))
	{
		// TODO @gdemers Update this later to reduce allocation made by TInstancedStruct.
		Projectile->ExplosionTemplate = TInstancedStruct<FExplosionParams>::Make(*this);
	}
}

#if WITH_EDITOR
EDataValidationResult UProjectileDefinitionDataAsset::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (!ProjectileParams.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UProjectileDefinitionDataAsset", "", "TInstancedStruct<FProjectileParams> is Null. No valid entries detected!"));
	}
	else if (bDoesExplode && !ExplosionParams.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UProjectileDefinitionDataAsset", "", "TInstancedStruct<FExplosionParams> is Null. No valid entries detected!"));
	}

	return Result;
}
#endif

const TSoftClassPtr<ANonReplicatedProjectileActor> UProjectileDefinitionDataAsset::GetProjectileClass() const
{
	const auto* Params = ProjectileParams.GetPtr<FProjectileParams>();
	return (Params != nullptr) ? Params->ProjectileClass : nullptr;
}

const TSoftClassPtr<ANonReplicatedExplosionActor> UProjectileDefinitionDataAsset::GetExplosionClass() const
{
	const auto* Params = ExplosionParams.GetPtr<FExplosionParams>();
	return (Params != nullptr) ? Params->ExplosionClass : nullptr;
}

const TInstancedStruct<FExplosionParams>& UProjectileDefinitionDataAsset::GetExplosionParams() const
{
	return ExplosionParams;
}

const TInstancedStruct<FProjectileParams>& UProjectileDefinitionDataAsset::GetProjectileParams() const
{
	return ProjectileParams;
}

const FGameplayTag& UProjectileDefinitionDataAsset::GetProjectileFiringModeTag() const
{
	return ProjectileFiringMode;
}

#if WITH_EDITOR
EDataValidationResult FProjectileDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ProjectileDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FProjectileDefinitionDataTableRow", "", "UProjectileDefinitionDataAsset missing. No valid UDataAsset specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FProjectileDefinitionDataTableRow::GetResourcesPaths() const
{
	return {ProjectileDefinition.ToSoftObjectPath()};
}
