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
#include "Tags/PrivateTags.h"

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
	
	DOREPLIFETIME_WITH_PARAMS_FAST(AWeaponActor_Range, CurrentFiringMode, FDoRepLifetimeParams());
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
	const bool bUseMeleeMode = CurrentFiringMode.MatchesAnyExact(FGameplayTagContainer{TAG_WEAPONSAMPLE_TRIGGER_TYPE_MELEE});
	if (bUseMeleeMode)
	{
		// @gdemers from an attachment hook onto your weapon.
		// this is different from the buttstock ability which require usage of a unique keybinding.
		MeleeTrigger();
	}
	else
	{
		// @gdemers either running from an attachment such as a grenade launcher, or
		// the normal mode the weapon use. example : full auto, vs single shot, vs burst.
		RangeTrigger();
	}
}

void AWeaponActor_Range::ToggleFiringMode(const FGameplayTag& NewFiringMode)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(AWeaponActor_Range, CurrentFiringMode, this);
	CurrentFiringMode = NewFiringMode;
}

void AWeaponActor_Range::MeleeTrigger_Implementation() const
{
	// TODO @gdemers Define what melee triggger imply at this level.
	// i.e those the attachment have durability ? BP impl will handle the VFX/and FX. 
}

void AWeaponActor_Range::RangeTrigger_Implementation() const
{
	const UArrowComponent* ProxyComponent = WeaponProxyComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(ProxyComponent), TEXT("Missing Proxy Component")))
	{
		return;
	}

	if (IsValid(ProjectileComponent))
	{
		// @gdemers CurrentFiringMode define the projectile type fired,
		// and/or alternate animation sequence we execute.
		ProjectileComponent->Fire(CurrentFiringMode, ProxyComponent->GetComponentTransform());
	}
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
