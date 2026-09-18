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

#include "AbilitySystemBlueprintLibrary.h"
#include "AttachmentManagerComponent.h"
#include "AVVMCharacter.h"
#include "AVVMLogger.h"
#include "AVVMReplicatedTagComponent.h"
#include "ProjectileComponent.h"
#include "ProjectileManagerSubsystem.h"
#include "WeaponSampleModule.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(AWeaponActor_Range, FiringModeGameplayEffectSpec, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AWeaponActor_Range, CurrentFiringMode, Params);
}

void AWeaponActor_Range::BeginPlay()
{
	Super::BeginPlay();

#if WITH_SERVER_CODE
	if (HasAuthority() && IsValid(ReplicatedTagComponent))
	{
		ReplicatedTagComponent->OnReplicatedTagChanged.AddUniqueDynamic(this, &AWeaponActor_Range::OnAvailableFiringModeCollectionChange);
	}
#endif
}

void AWeaponActor_Range::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_SERVER_CODE
	if (HasAuthority() && IsValid(ReplicatedTagComponent))
	{
		ReplicatedTagComponent->OnReplicatedTagChanged.RemoveAll(this);
	}
#endif
}

void AWeaponActor_Range::Trigger_Implementation() const
{
	const bool bUseMeleeMode = CurrentFiringMode.MatchesAnyExact(FGameplayTagContainer{TAG_WEAPONSAMPLE_TRIGGER_TYPE_MELEE});
	if (bUseMeleeMode)
	{
		// @gdemers execute a melee attack based on attachment hook onto weapon.
		// IMPORTANT - This isnt a buttstroke action. Buttstroke would be set to a unique keybinding, while the current trigger action
		// shares the same input key as default firing, and execute extended behaviour based on the firing mode.
		MeleeTrigger();
	}
	else
	{
		// @gdemers execute a range attack. projectile specific information is defined based on currently selected
		// Firing mode which extend into :
		// Light Rounds, Heavy Rounds, incendiary, missiles, etc...
		// IMPORTANT - The fired projectile is managed via GameplayEffect. 
		RangeTrigger();
	}
}

void AWeaponActor_Range::ToggleFiringMode()
{
	if (!IsValid(ReplicatedTagComponent))
	{
		return;
	}

	TArray<FGameplayTag> OutTags{};
	ReplicatedTagComponent->GetRuntimeTagsArray(OutTags);

	int32 CurrIndex = OutTags.IndexOfByKey(CurrentFiringMode);
	if (ensureAlwaysMsgf(CurrIndex != INDEX_NONE,
	                     TEXT("Invalid Tag lookup.")))
	{
		CurrIndex = ((CurrIndex + 1) % OutTags.Num());
		SetFiringMode(OutTags[CurrIndex]);
	}
}

void AWeaponActor_Range::RangeTrigger_Implementation() const
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	const UArrowComponent* ProxyComponent = GetMutableAimingComponent();
	if (!ensureAlwaysMsgf(IsValid(ProxyComponent),
						  TEXT("Missing Proxy Component")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
					this,
					Outer,
					TEXT("Trigger"));

	if (IsValid(ProjectileComponent))
	{
		// @gdemers CurrentFiringMode may refer to Default, Light, Heavy Rounds, or even more advance
		// projectile types such as grenade launcher, or missiles.
		ProjectileComponent->Fire(FiringModeGameplayEffectSpec, ProxyComponent->GetComponentTransform());
	}
}

void AWeaponActor_Range::MeleeTrigger_Implementation() const
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                this,
	                Outer,
	                TEXT("Melee Trigger"));

	FGameplayEventData GAS_EventData{};
	GAS_EventData.Instigator = this;
	GAS_EventData.Target = Outer;
	// @gdemers Play montage for melee ability
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(const_cast<AActor*>(Outer), TAG_WEAPONSAMPLE_TRIGGER_TYPE_MELEE, MoveTemp(GAS_EventData));
}

void AWeaponActor_Range::SetFiringMode(const FGameplayTag& NewFiringMode)
{
	if (!IsValid(ReplicatedTagComponent) || !ensureAlwaysMsgf(ReplicatedTagComponent->HasAnyExactRuntimeTags(FGameplayTagContainer{NewFiringMode}),
	                                                          TEXT("Invalid Firing Mode. Mode not supported.")))
	{
		return;
	}

	// @gdemers set the active projectile type. example : light rounds, heavy rounds, incendiary, etc...
	ApplyFiringModeGameplayEffect(NewFiringMode);
	MARK_PROPERTY_DIRTY_FROM_NAME(AWeaponActor_Range, CurrentFiringMode, this);
	CurrentFiringMode = NewFiringMode;
}

void AWeaponActor_Range::ApplyFiringModeGameplayEffect(const FGameplayTag& NewFiringMode)
{
	auto* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	ASC->RemoveActiveGameplayEffect(NonReplicatedFiringModeActiveGameplayEffectHandle);
	// @gdemers Allowed to fail when the firing mode doesnt support projectiles. example : bayonet.
	const TSoftClassPtr<UGameplayEffect> FiringModeGameplayEffectClass = UProjectileManagerSubsystem::Static_GetFiringModeGameplayEffectClass(GetWorld(), NewFiringMode);
	if (FiringModeGameplayEffectClass.IsNull())
	{
		return;
	}

	const auto SpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(FiringModeGameplayEffectClass.LoadSynchronous(), const_cast<AActor*>(OwningOuter.Get()), this);
	NonReplicatedFiringModeActiveGameplayEffectHandle = ASC->BP_ApplyGameplayEffectSpecToSelf(SpecHandle);

	if (SpecHandle.Data.IsValid())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AWeaponActor_Range, FiringModeGameplayEffectSpec, this);
		FiringModeGameplayEffectSpec = *SpecHandle.Data.Get();
	}
}

const UArrowComponent* AWeaponActor_Range::GetMutableAimingComponent() const
{
	if (!WeaponProxyComponent.IsValid() && OwningOuter.IsValid())
	{
		//@gdemers mutable since we suffer from race conditions on BeginPlay, and require latent caching
		// of the member value. Why ? remove the need of violating constness.
		const auto* Outer = Cast<AAVVMCharacter>(OwningOuter);
		WeaponProxyComponent = IsValid(Outer) ? Outer->GetAimingComponent() : nullptr;
	}

	return WeaponProxyComponent.Get();
}

void AWeaponActor_Range::OnAvailableFiringModeCollectionChange(const FGameplayTagContainer& NewTags)
{
	if (!CurrentFiringMode.IsValid() && ensureAlwaysMsgf(NewTags.IsValid(), TEXT("Invalid Tags")))
	{
		SetFiringMode(NewTags.First());
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
