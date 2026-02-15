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

#include "AVVMPlayerState.h"
#include "NonReplicatedProjectileActor.h"
#include "Data/ProjectileDefinitionDataAsset.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Tags/AVVMGameplayTags.h"

bool UProjectileManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (!IsValid(World))
	{
		return false;
	}

	return World->IsGameWorld();
}

void UProjectileManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ClientPlayerController.Reset();

	auto* GameStateBase = UGameplayStatics::GetGameState(this);
	if (!IsValid(GameStateBase))
	{
		return;
	}

	FAVVMOnChannelNotifiedSingleCastDelegate Callback;
	Callback.BindDynamic(this, &UProjectileManagerSubsystem::OnPlayerStateAddedOrRemoved);

	FAVVMObserverContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_ONPLAYERSTATE_ADDED_OR_REMOVED;
	ContextArgs.Callback = Callback;
	UAVVMNotificationSubsystem::Static_RegisterObserver(GameStateBase, ContextArgs);

	for (const TObjectPtr<APlayerState>& PlayerState : GameStateBase->PlayerArray)
	{
		OnPlayerStateAdded(PlayerState);
	}
}

void UProjectileManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ClientPlayerController.Reset();
	Projectiles.Reset();

	auto* GameStateBase = UGameplayStatics::GetGameState(this);
	if (IsValid(GameStateBase))
	{
		FAVVMObserverContextArgs ContextArgs;
		ContextArgs.ChannelTag = TAG_AVVMGAMEPLAY_GAMESTATE_ONPLAYERSTATE_ADDED_OR_REMOVED;
		UAVVMNotificationSubsystem::Static_UnregisterObserver(GameStateBase, ContextArgs);
	}
}

void UProjectileManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TStatId UProjectileManagerSubsystem::GetStatId() const
{
	return TStatId();
}

void UProjectileManagerSubsystem::Static_Register(const UWorld* World,
                                                  ANonReplicatedProjectileActor* Projectile)
{
	auto* ProjectileSubsystem = UProjectileManagerSubsystem::Get(World);
	if (IsValid(ProjectileSubsystem))
	{
		ProjectileSubsystem->Register(Projectile);
	}
}

void UProjectileManagerSubsystem::Static_Unregister(const UWorld* World,
                                                    ANonReplicatedProjectileActor* Projectile)
{
	auto* ProjectileSubsystem = UProjectileManagerSubsystem::Get(World);
	if (IsValid(ProjectileSubsystem))
	{
		ProjectileSubsystem->Unregister(Projectile);
	}
}

void UProjectileManagerSubsystem::Static_CreateProjectile(const UWorld* World,
                                                          const FProjectileContextArgs& ContextArgs)
{
	auto* ProjectileSubsystem = UProjectileManagerSubsystem::Get(World);
	if (IsValid(ProjectileSubsystem))
	{
		ProjectileSubsystem->CreateProjectile(ContextArgs);
	}
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

void UProjectileManagerSubsystem::CreateProjectile(const FProjectileContextArgs& ContextArgs) const
{
	const auto* Params = ContextArgs.ProjectileParams.GetPtr<FProjectileParams>();
	if (Params != nullptr)
	{
		ANonReplicatedProjectileActor* Instance = Factory(ContextArgs.ProjectileClass, ContextArgs.AimTransform);
		Params->Init(Instance);
	}
}

void UProjectileManagerSubsystem::OnPlayerStateAddedOrRemoved(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
	const auto* Payload = NewPayload.GetPtr<FAVVMPlayerStatePayload>();
	if (!ensureAlwaysMsgf(Payload != nullptr,
						  TEXT("Payload couldnt be casted to FAVVMPlayerStatePayload type")))
	{
		return;
	}

	const bool bWasAddedOrRemoved = Payload->bWasAddedOrRemoved;
	if (bWasAddedOrRemoved)
	{
		OnPlayerStateAdded(Payload->PlayerState.Get());
	}
	else
	{
		OnPlayerStateRemoved(Payload->PlayerState.Get());
	}
}

UProjectileManagerSubsystem* UProjectileManagerSubsystem::Get(const UWorld* World)
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
