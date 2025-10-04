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

#include "GameplayTagContainer.h"
#include "Data/AVVMDataTableRow.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "ProjectileDefinitionDataAsset.generated.h"

class ANonReplicatedExplosionActor;
class ANonReplicatedProjectileActor;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<ANonReplicatedProjectileActor> ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Speed = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Mass = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Lifetime = 0.f;
};

/**
 *	Class description:
 *
*	FExplosionParams is a context struct that encapsulate properties specific to an explosion
 *	and offer user extensibility through usage of TInstancedStruct<T>.
 */
USTRUCT(BlueprintType)
struct FExplosionParams
{
	GENERATED_BODY()

	virtual ~FExplosionParams() = default;
	virtual void Init(ANonReplicatedProjectileActor* Projectile) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<ANonReplicatedExplosionActor> ExplosionClass = nullptr;
};

/**
 *	Class description:
 *
 *	UProjectileDefinitionDataAsset is a POD asset that defines the properties of a Projectile Actor.
 */
UCLASS()
class WEAPONSAMPLE_API UProjectileDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	const TSoftClassPtr<ANonReplicatedProjectileActor> GetProjectileClass() const;
	const TSoftClassPtr<ANonReplicatedExplosionActor> GetExplosionClass() const;
	const TInstancedStruct<FExplosionParams>& GetExplosionParams() const;
	const TInstancedStruct<FProjectileParams>& GetProjectileParams() const;
	const FGameplayTag& GetProjectileFiringModeTag() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag ProjectileFiringMode = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TInstancedStruct<FProjectileParams> ProjectileParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bDoesExplode = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bDoesExplode"))
	TInstancedStruct<FExplosionParams> ExplosionParams;
};

/**
 *	Class description:
 *
 *	FProjectileDefinitionDataTableRow is an entry in a DataTableRow for a unique UProjectileDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FProjectileDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UProjectileDefinitionDataAsset> ProjectileDefinition = nullptr;
};
