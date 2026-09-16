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

#include "GameplayEffect.h"
#include "Data/ProjectileDefinitionDataAsset.h"
#include "StructUtils/InstancedStruct.h"

#include "GameplayEffect_ProjectileTemplate.generated.h"

class ANonReplicatedProjectileActor;

/**
 *	Class description:
 *	
 *	FFiringModeSparseData is a Shared representation of a class object immutable data. It reduces memory footprint
 *	by removing the need to allocate that data on instanced class object, and instead reference the shared memory.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FFiringModeSparseData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	TSoftClassPtr<ANonReplicatedProjectileActor> ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	TInstancedStruct<FProjectileParams> ProjectileParams{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	bool bDoesExplode{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef, EditCondition="bDoesExplode"))
	TInstancedStruct<FExplosionParams> ExplosionParams{};
};

/**
 *	Class description:
 *	
 *	UGameplayEffect_FiringMode is a GameplayEffect context applied to a Triggering Actor that mutate the
 *	triggering mode, and the output generated from the trigger event (VFX, audio, etc...)
 *	
 *	example : when triggering in `BURST` mode, we may use Heavy rounds, but in automatic mode, we may be
 *	using Light rounds.
 *	
 *	Using gameplay effect allow AttachmentActors to register Gameplay Tags that are `ALLOWED`
 *	firing modes on the bound weapon without requiring storing projectiles information on the attachment OR
 *	the weapon actor.
 */
UCLASS(Abstract, Blueprintable, SparseClassDataTypes="FiringModeSparseData")
class WEAPONSAMPLE_API UGameplayEffect_FiringMode : public UGameplayEffect
{
	GENERATED_BODY()
};
