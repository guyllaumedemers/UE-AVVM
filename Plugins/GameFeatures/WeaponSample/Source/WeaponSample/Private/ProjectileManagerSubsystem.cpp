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

#include "AVVMGameState.h"
#include "NonReplicatedProjectileActor.h"
#include "WeaponDebuggerSettings.h"
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
	
	if (World->IsNetMode(NM_DedicatedServer))
	{
		HandleClientPassByBullets();
	}
}

void UProjectileManagerSubsystem::Register(ANonReplicatedProjectileActor* Projectile)
{
	if (IsValid(Projectile))
	{
		Projectile->OnProjectilePoolingRequest.AddUObject(this, &UProjectileManagerSubsystem::OnProjectileRequestPooling);
		Projectiles.Add(Projectile);
	}
}

void UProjectileManagerSubsystem::Unregister(ANonReplicatedProjectileActor* Projectile)
{
	if (IsValid(Projectile))
	{
		Projectile->OnProjectilePoolingRequest.RemoveAll(this);
		Projectiles.RemoveSwap(Projectile);
	}
}

UProjectileManagerSubsystem* UProjectileManagerSubsystem::GetSubsystem(const UWorld* World)
{
	return UWorld::GetSubsystem<UProjectileManagerSubsystem>(World);
}

void UProjectileManagerSubsystem::OnPlayerStateRemoved(APlayerState* PlayerState)
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

void UProjectileManagerSubsystem::OnPlayerStateAdded(APlayerState* PlayerState)
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

void UProjectileManagerSubsystem::OnProjectileRequestPooling(ANonReplicatedProjectileActor* Projectile)
{
	// TODO @gdemers impl pooling system
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
		if (!Iterator->IsValid())
		{
			Iterator.RemoveCurrentSwap();
			continue;
		}

		const bool bIsWithinRange = UProjectileFunctionLibrary::CheckDistance(Iterator->Get(),
																			  Pawn,
																			  UWeaponDebuggerSettings::GetSquaredDistanceThreshold());
		if (!bIsWithinRange)
		{
			continue;
		}

		const FTransform& ProjectileWorldTransform = Iterator->Get()->GetTransform();
		OnProjectilePassBy.Broadcast(ProjectileWorldTransform.GetLocation());
		Iterator.RemoveCurrentSwap();
	}
}
