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
#include "ProjectileComponent.h"

#include "AVVMLogger.h"
#include "NonReplicatedProjectileActor.h"
#include "ProjectileManagerSubsystem.h"
#include "Effect/GameplayEffect_FiringMode.h"

UProjectileComponent::UProjectileComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(false);
}

void UProjectileComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningOuter = GetTypedOuter<AActor>();
}

void UProjectileComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	OwningOuter.Reset();
}

void UProjectileComponent::Fire(const FGameplayEffectSpecHandle& FiringModeGameplayEffectSpecHandle,
                                const FTransform& AimTransform) const
{
	if (!ensureAlwaysMsgf(FiringModeGameplayEffectSpecHandle.Data.IsValid(),
	                      TEXT("Invalid Spec Handle.")))
	{
		return;
	}

	const auto* FiringModeGameplayEffect = Cast<UGameplayEffect_FiringMode>(FiringModeGameplayEffectSpecHandle.Data->Def);
	if (!ensureAlwaysMsgf(IsValid(FiringModeGameplayEffect), TEXT("Invalid Projectile GameplayEffect.")))
	{
		return;
	}

	FProjectileContextArgs ContextArgs;
	ContextArgs.Owner = OwningOuter.Get();
	ContextArgs.ProjectileClass = FiringModeGameplayEffect->GetProjectileClass().LoadSynchronous(); // TODO @gdemers handle the async approach.
	ContextArgs.ProjectileParams = FiringModeGameplayEffect->GetProjectileParams();
	ContextArgs.AimTransform = AimTransform;
	ContextArgs.IgnoredActors =
	{
			const_cast<AActor*>(OwningOuter.Get()),
			OwningOuter->GetTypedOuter<AActor>()
	};

	UProjectileManagerSubsystem::Static_CreateProjectile(GetWorld(), ContextArgs);
}
