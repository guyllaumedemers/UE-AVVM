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

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AVVMGameplayUtils.h"
#include "TriggeringAbility.h"
#include "WeaponSample.h"
#include "Ability/AVVMGameplayAbility.h"
#include "Engine/AssetManager.h"

void ATriggeringActor::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
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
	if (bShouldAsyncLoadOnBeginPlay)
	{
		Swap(true);
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
	Swap(false);
#endif
}

void ATriggeringActor::Swap_Implementation(const bool bIsActive)
{
	UnRegisterAbility();

	if (bIsActive)
	{
		RegisterAbility();
	}
}

void ATriggeringActor::RegisterAbility()
{
	FStreamableDelegate OnRequestTriggeringActorAbilityComplete;
	OnRequestTriggeringActorAbilityComplete.BindUObject(this, &ATriggeringActor::OnSoftObjectAcquired);
	TriggeringAbilityClassHandle = UAssetManager::Get().LoadAssetList({TriggeringAbilityClass.ToSoftObjectPath()});
}

void ATriggeringActor::UnRegisterAbility()
{
	if (!TriggeringAbility.IsValid())
	{
		return;
	}

	auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningOuter.Get());
	if (IsValid(ASC))
	{
		ASC->ClearAbility(TriggeringAbility->GetCurrentAbilitySpecHandle());
	}
}

void ATriggeringActor::OnSoftObjectAcquired()
{
	auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningOuter.Get());
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

	ASC->GiveAbility(FGameplayAbilitySpec{
		GameplayAbilityClass,
		1,
		GameplayAbilityClass->GetDefaultObject<UAVVMGameplayAbility>()->GetInputId()
	});
}
