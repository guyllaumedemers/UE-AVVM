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

#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"
#include "StructUtils/InstancedStruct.h"

#include "ProjectileComponent.generated.h"

struct FExplosionParams;
struct FProjectileParams;
class UProjectileManagerSubsystem;

/**
 *	Class description:
 *
 *	UProjectileComponent is a system handling projectile instantiation, and initialization.
 */
UCLASS(ClassGroup=("Weapon"), Blueprintable, meta=(BlueprintSpawnableComponent))
class WEAPONSAMPLE_API UProjectileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	static UProjectileComponent* GetActorComponent(const AActor* NewActor);

	UFUNCTION(BlueprintCallable)
	void Fire(const FGameplayTag& FiringModeTag,
	          const FTransform& AimTransform) const;

	void SetupProjectiles(const TArray<UObject*>& NewResources);

protected:
	UFUNCTION()
	void OnSoftObjectAcquired();
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	struct FProjectileFiringMode
	{
		TInstancedStruct<const FProjectileParams> ProjectileParams;
		TWeakObjectPtr<const UClass> ProjectileClass = nullptr;
	};

	TWeakObjectPtr<UProjectileManagerSubsystem> ProjectileManagerSubsystem = nullptr;
	TMap<FGameplayTag, FProjectileFiringMode> ProjectileTemplates;
	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};
