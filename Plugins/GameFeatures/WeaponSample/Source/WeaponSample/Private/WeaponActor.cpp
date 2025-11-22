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
#include "WeaponActor.h"

#include "AttachmentManagerComponent.h"
#include "ProjectileComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AWeaponActor_Range::AWeaponActor_Range(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AttachmentManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAttachmentManagerComponent>(this, TEXT("AttachmentManagerComponent"));
	SkeletalMeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("SkeletalMeshComponent"));
	ProjectileComponent = ObjectInitializer.CreateDefaultSubobject<UProjectileComponent>(this, TEXT("ProjectileComponent"));
}

void AWeaponActor_Range::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeaponActor_Range, CurrentFiringMode);
}

void AWeaponActor_Range::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<ACharacter>();
	if (IsValid(Outer))
	{
		WeaponProxyComponent = Outer->GetComponentByClass<UArrowComponent>();
	}
}

void AWeaponActor_Range::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AWeaponActor_Range::Trigger_Implementation() const
{
	const UArrowComponent* ProxyComponent = WeaponProxyComponent.Get();
	if (!IsValid(ProxyComponent))
	{
		return;
	}

	if (IsValid(ProjectileComponent))
	{
		// TODO @gdemers Require more work to better define Client-Server-OtherClient interaction.
		// check Unreal ShooterGame.
		ProjectileComponent->Fire(CurrentFiringMode, ProxyComponent->GetComponentTransform());
	}
}

void AWeaponActor_Range::ToggleFiringMode(const FGameplayTag& NewFiringMode)
{
	// TODO @gdemers Do impl.
}

AWeaponActor_Melee::AWeaponActor_Melee(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SkeletalMeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("SkeletalMeshComponent"));
}

void AWeaponActor_Melee::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponActor_Melee::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AWeaponActor_Melee::Trigger_Implementation() const
{
}
