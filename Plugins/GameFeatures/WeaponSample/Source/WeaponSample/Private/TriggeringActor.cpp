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
#include "TriggeringActor.h"

#include "AVVMGameplayUtils.h"
#include "AVVMUtils.h"
#include "WeaponSample.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Ability/AVVMGameplayAbility.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Resources/AVVMResourceManagerComponent.h"

ATriggeringActor::ATriggeringActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAbilitySystemComponent>(this, TEXT("MAbilitySystemComponent"));
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));

	// @gdemers if tick is required, start an AbilityTask_Tick, and kill the process on completion.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickBatching = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	SetReplicateMovement(true);
	bReplicates = true;
}

void ATriggeringActor::BeginPlay()
{
	Super::BeginPlay();

	auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	OwningOuter = Outer;

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *ATriggeringActor::StaticClass()->GetName());

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Outer);
		if (ensureAlwaysMsgf(UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral),
		                     TEXT("Outer doesn't implement required interface.")))
		{
			SocketDeferral->NotifyAvailableSocketParent(this);
		}

		if (bShouldAsyncLoadOnBeginPlay)
		{
			Server_SwapAbility(true);
		}
	}
#endif
}

void ATriggeringActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *ATriggeringActor::StaticClass()->GetName());

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		Server_SwapAbility(false);
	}
#endif
}

void ATriggeringActor::Server_SwapAbility_Implementation(const bool bIsActive)
{
	UnRegisterAbility();

	if (bIsActive)
	{
		RegisterAbility();
	}
}

UAbilitySystemComponent* ATriggeringActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAVVMResourceManagerComponent* ATriggeringActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> ATriggeringActor::GetResourceDefinitionResourceIds_Implementation() const
{
	return {TriggeringDefinitionId};
}

void ATriggeringActor::RegisterAbility()
{
	// @gdemers IMPORTANT : we are not passing through the AVVMResourceManagerComponent here to async load the GameplayAbility class.
	// Doing so would prevent caching of the Ability and removal of it during context switching of triggering actors. (i.e during weapon switch, etc...)
	FStreamableDelegate OnRequestTriggeringActorAbilityComplete;
	OnRequestTriggeringActorAbilityComplete.BindUObject(this, &ATriggeringActor::OnTriggeringAbilityClassAcquired);
	TriggeringAbilityClassHandle = UAssetManager::Get().LoadAssetList({TriggeringAbilityClass.ToSoftObjectPath()});
}

void ATriggeringActor::UnRegisterAbility()
{
	if (!TriggeringAbilitySpecHandle.IsValid())
	{
		return;
	}

	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
	if (IsValid(ASC))
	{
		ASC->ClearAbility(TriggeringAbilitySpecHandle);
	}
}

void ATriggeringActor::OnTriggeringAbilityClassAcquired()
{
	auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(OwningOuter.Get());
	if (!TriggeringAbilityClassHandle.IsValid() || !ensureAlwaysMsgf(IsValid(ASC),
	                                                                 TEXT("Owning Outer missing valid ASC.")))
	{
		return;
	}
	
	TArray<UObject*> OutStreamableAssets;
	TriggeringAbilityClassHandle->GetLoadedAssets(OutStreamableAssets);

	if (OutStreamableAssets.IsEmpty())
	{
		return;
	}

	auto* GameplayAbilityClass = Cast<UClass>(OutStreamableAssets[0]);
	if (!IsValid(GameplayAbilityClass))
	{
		return;
	}

	TriggeringAbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec{
		GameplayAbilityClass,
		1,
		GameplayAbilityClass->GetDefaultObject<UAVVMGameplayAbility>()->GetInputId()
	});
}

void UTriggeringUtils::Swap(ATriggeringActor* UnEquip,
                            ATriggeringActor* Equip)
{
	if (IsValid(UnEquip))
	{
		UnEquip->Server_SwapAbility(false);
	}

	if (IsValid(Equip))
	{
		Equip->Server_SwapAbility(true);
	}
}
