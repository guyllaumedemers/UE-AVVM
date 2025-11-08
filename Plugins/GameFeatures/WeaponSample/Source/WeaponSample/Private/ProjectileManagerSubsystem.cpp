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
#include "ProjectileManagerSubsystem.h"

#include "AVVMAudioContext.h"
#include "AVVMAudioUtils.h"
#include "AVVMGameState.h"
#include "NonReplicatedProjectileActor.h"
#include "WeaponSettings.h"
#include "Data/ProjectileDefinitionDataAsset.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UProjectileManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto* GameState = Cast<AAVVMGameState>(UGameplayStatics::GetGameState(this));
	if (!IsValid(GameState))
	{
		return;
	}

	GameState->OnPlayerStateRemoved.AddUObject(this, &UProjectileManagerSubsystem::OnPlayerStateRemoved);
	GameState->OnPlayerStateAdded.AddUObject(this, &UProjectileManagerSubsystem::OnPlayerStateAdded);

	ClientPlayerController.Reset();
	for (const TObjectPtr<APlayerState>& PlayerState : GameState->PlayerArray)
	{
		OnPlayerStateAdded(PlayerState);
	}
}

void UProjectileManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ClientPlayerController.Reset();
	Projectiles.Reset();

	auto* GameState = Cast<AAVVMGameState>(UGameplayStatics::GetGameState(this));
	if (IsValid(GameState))
	{
		GameState->OnPlayerStateRemoved.RemoveAll(this);
		GameState->OnPlayerStateAdded.RemoveAll(this);
	}
}

void UProjectileManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	if (!World->IsNetMode(NM_DedicatedServer))
	{
		HandleClientPassByBullets();
	}
}

TStatId UProjectileManagerSubsystem::GetStatId() const
{
	return TStatId();
}

void UProjectileManagerSubsystem::Register(ANonReplicatedProjectileActor* Projectile)
{
	if (IsValid(Projectile))
	{
		Projectile->OnProjectileShutdown.AddUObject(this, &UProjectileManagerSubsystem::OnProjectileShutdownRequested);
		Projectiles.Add(Projectile);
	}
}

void UProjectileManagerSubsystem::Unregister(ANonReplicatedProjectileActor* Projectile)
{
	if (IsValid(Projectile))
	{
		Projectile->OnProjectileShutdown.RemoveAll(this);
		Projectiles.RemoveSwap(Projectile);
	}
}

void UProjectileManagerSubsystem::CreateProjectile(const UClass* ProjectileClass,
                                                   const TInstancedStruct<const FProjectileParams>& ProjectileParams,
                                                   const FTransform& AimTransform) const
{
	const auto* Params = ProjectileParams.GetPtr<FProjectileParams>();
	if (Params != nullptr)
	{
		ANonReplicatedProjectileActor* Instance = Factory(ProjectileClass, AimTransform);
		Params->Init(Instance);
	}
}

UProjectileManagerSubsystem* UProjectileManagerSubsystem::GetSubsystem(const UWorld* World)
{
	return UWorld::GetSubsystem<UProjectileManagerSubsystem>(World);
}

void UProjectileManagerSubsystem::OnPlayerStateRemoved(const APlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APlayerController* PC = PlayerState->GetPlayerController();
	if (IsValid(PC) && PC->IsLocalPlayerController())
	{
		ClientPlayerController.Reset();
	}
}

void UProjectileManagerSubsystem::OnPlayerStateAdded(const APlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		return;
	}

	const APlayerController* PC = PlayerState->GetPlayerController();
	if (IsValid(PC) && PC->IsLocalPlayerController())
	{
		ClientPlayerController = PC;
	}
}

ANonReplicatedProjectileActor* UProjectileManagerSubsystem::Factory(const UClass* ProjectileClass, const FTransform& AimTransform) const
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const FVector Location = AimTransform.GetLocation();
		const FRotator Rotator = AimTransform.Rotator();
		// @gdemers another good example of const-ness issue in the engine. theres no point in violating bitwise const-ness or logical const-ness and yet
		// the engine require a UClass* to be non-const.
		AActor* Instance = World->SpawnActor(const_cast<UClass*>(ProjectileClass), &Location, &Rotator, FActorSpawnParameters());
		return Cast<ANonReplicatedProjectileActor>(Instance);
	}

	return nullptr;
}

void UProjectileManagerSubsystem::Shutdown(ANonReplicatedProjectileActor* Projectile)
{
	if (IsValid(Projectile))
	{
		Projectile->Destroy();
	}
}

void UProjectileManagerSubsystem::OnProjectileShutdownRequested(ANonReplicatedProjectileActor* Projectile)
{
	Shutdown(Projectile);
}

void UProjectileManagerSubsystem::HandleClientPassByBullets()
{
	if (!ClientPlayerController.IsValid())
	{
		return;
	}

	const APawn* Pawn = ClientPlayerController->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	for (auto Iterator = Projectiles.CreateIterator(); Iterator; ++Iterator)
	{
		if (!IsValid(*Iterator))
		{
			Iterator.RemoveCurrentSwap();
			continue;
		}

		const bool bIsWithinRange = UProjectileFunctionLibrary::CheckDistance(Iterator->Get(),
																			  Pawn,
																			  UWeaponSettings::GetSquaredDistanceThreshold());
		if (bIsWithinRange)
		{
			const TInstancedStruct<FAVVMAudioContext> SpatialAudioContext = FAVVMAudioContext::Make<FAVVMSpatialAudioContext>(Iterator->Get(), Pawn, 1.f);
			UAVVMAudioUtils::NotifyAudioCue(this, SpatialAudioContext);
			Iterator.RemoveCurrentSwap();
		}
	}
}
