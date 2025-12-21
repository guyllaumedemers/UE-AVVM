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
#include "GameFramework/Character.h"
#include "Resources/AVVMResourceManagerComponent.h"

TArray<int32> FTriggeringActorDataResolverHelper::GetElementDependencies(const UObject* WorldContextObject, const int32 ElementId) const
{
	// TODO @gdemers retrieve a global access of the relevant data.
	return TArray<int32>();
}

AActor* FTriggeringSocketTargetingHelper::GetDesiredTypedInner(AActor* Src, AActor* Target) const
{
	if (!IsValid(Target))
	{
		return nullptr;
	}

	auto* Pawn = Cast<ACharacter>(Target);
	return IsValid(Pawn) ? Pawn : nullptr;
}

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
		IAVVMDoesSupportInnerSocketTargeting::Execute_Detach(this);
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

void ATriggeringActor::SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet)
{
	OwnedAttributeSet = NewAttributeSet;
}

TInstancedStruct<FAVVMSocketTargetingHelper> ATriggeringActor::GetSocketHelper_Implementation() const
{
	return FAVVMSocketTargetingHelper::Make<FTriggeringSocketTargetingHelper>();
}

void ATriggeringActor::DeferredSocketParenting_Implementation(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	AActor* Parent = ContextArgs.Parent.Get();
	if (!IsValid(Parent))
	{
		return;
	}

	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Parent);

	const bool bDoesImplement = UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
						  TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	IAVVMDoesSupportSocketDeferral::FOnParentSocketAvailableDelegate::FDelegate Callback;
	Callback.BindUObject(this, &ATriggeringActor::OnSocketParentingDeferred, ContextArgs);
	DeferredSocketParentingDelegateHandle = SocketDeferral->OnSocketParentAvailableDelegate_Add(Callback);
}

void ATriggeringActor::Attach_Implementation(AActor* Target, const FName NewSocketName)
{
	if (!ensureAlwaysMsgf(IsValid(Target), TEXT("Invalid Parent!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
		   Log,
		   TEXT("Executed from \"%s\". Attaching \"%s\" to Outer \"%s\" at SocketName \"%s\"."),
		   UAVVMGameplayUtils::PrintNetSource(Target).GetData(),
		   *ATriggeringActor::StaticClass()->GetName(),
		   *Target->GetName(),
		   *NewSocketName.ToString());

	// @gdemers detach actor + remove AttributeSet registered
	IAVVMDoesSupportInnerSocketTargeting::Execute_Detach(this);

	// @gdemers attach actor + add AttributeSet
	AttachToActor(Target, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName);
	OwningOuter = Target;

	// @gdemers Unregister/Register ability from owner.
	Server_SwapAbility(true);

	// @gdemers attempt registering AttributeSet with ASC. may fail but thats alright! the inventory system handle that case.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->RegisterAttributeSet(OwnedAttributeSet, this);
	}
}

void ATriggeringActor::Detach_Implementation()
{
	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Detaching \"%s\" from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *ATriggeringActor::StaticClass()->GetName(),
	       *Outer->GetName());

	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

	// @gdemers Unregister ability from owner.
	Server_SwapAbility(false);

	// @gdemers clear AttributeSet provided by this attachment.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (IsValid(ASC))
	{
		ASC->UnRegisterAttributeSet(this);
	}
}

UAVVMResourceManagerComponent* ATriggeringActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> ATriggeringActor::GetResourceDefinitionResourceIds_Implementation() const
{
	return {TriggeringDefinitionId};
}

const TInstancedStruct<FAVVMDataResolverHelper>& ATriggeringActor::GetTriggeringActorDataResolverHelper()
{
	static auto Helper = FAVVMDataResolverHelper::Make<FTriggeringActorDataResolverHelper>();
	return Helper;
}

void ATriggeringActor::OnSocketParentingDeferred(AActor* Parent,
                                                 AActor* Target,
                                                 const FAVVMSocketTargetingDeferralContextArgs ContextArgs)
{
	auto SocketDeferral = TScriptInterface<IAVVMDoesSupportSocketDeferral>(Parent);

	const bool bDoesImplement = UAVVMUtils::IsNativeScriptInterfaceValid(SocketDeferral);
	if (!ensureAlwaysMsgf(bDoesImplement,
	                      TEXT("Dest actor doesn't implement the required interface")))
	{
		return;
	}

	SocketDeferral->OnSocketParentAvailableDelegate_Remove(DeferredSocketParentingDelegateHandle);
	const bool bIsRooted = FAVVMSocketTargetingHelper::Static_AttachToActor(this, ContextArgs);
	if (!bIsRooted)
	{
		return;
	}

	// @gdemers Initialized the AttributeSet for the first time based on deferred socketing.
	auto* ASC = Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
	if (ensureAlwaysMsgf(IsValid(ASC),
	                     TEXT("New OwningOuter doesn't own a valid ASC.")))
	{
		ASC->SetupAttributeSet(ContextArgs.SrcAttributeSetSoftObjectPath, Target);
	}
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

void UTriggeringUtils::Swap(AActor* UnEquip,
                            AActor* Equip,
                            const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	FAVVMSocketTargetingHelper::Static_Detach(UnEquip);
	FAVVMSocketTargetingHelper::Static_AttachToActor(Equip, ContextArgs);
}
