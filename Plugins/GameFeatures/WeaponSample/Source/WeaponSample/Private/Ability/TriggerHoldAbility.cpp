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
#include "Ability/TriggerHoldAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AVVMLogger.h"
#include "TriggeringActor.h"
#include "WeaponSampleModule.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Ability/AVVMAbilityTask_TickUntil.h"
#include "Data/Weapon/WeaponAttributeSet.h"
#include "GameFramework/PlayerController.h"

void UTriggerHoldAbility::PreActivate(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	const auto* ASC = (ActorInfo != nullptr) ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ensureAlwaysMsgf(IsValid(ASC),
	                      TEXT("UTriggerAbility owning ASC invalid!")))
	{
		return;
	}

	const auto* GASpec = ASC->FindAbilitySpecFromHandle(Handle);
	if (!ensureAlwaysMsgf(GASpec != nullptr, TEXT("Failed to Find Ability Spec from Handle")))
	{
		return;
	}

	auto* TriggeringActor = Cast<ATriggeringActor>(GASpec->SourceObject);
	if (!ensureAlwaysMsgf(IsValid(TriggeringActor), TEXT("Invalid TriggeringActor.")))
	{
		return;
	}

	bool bOutResult = false;
	TriggerProgressRate = UAbilitySystemBlueprintLibrary::GetFloatAttribute(TriggeringActor,
	                                                                        GET_GAMEPLAY_ATTRIBUTE_USING_IILE(UWeaponRange_AttributeSet, TriggeringActor, RateOfFire),
	                                                                        bOutResult);

	ensureAlwaysMsgf(bOutResult, TEXT("Failed to retrieve GameplayAttribute."));
}

void UTriggerHoldAbility::RunOptionalTask(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	PlayerController = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;

	auto* ParentTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (IsValid(ParentTask))
	{
		ParentTask->OnRelease.AddUniqueDynamic(this, &UTriggerHoldAbility::OnInputReleased);
		ParentTask->ReadyForActivation();
	}

	auto* ChildTask = UAVVMAbilityTask_TickUntil::TickUntil(this, true);
	if (IsValid(ChildTask))
	{
		ChildTask->OnTick.AddUniqueDynamic(this, &UTriggerHoldAbility::OnTick);
		ChildTask->ReadyForActivation();
	}
}

void UTriggerHoldAbility::OnInputReleased(float TimeHeld)
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();

	const AActor* Controller = PlayerController.Get();
	if (!ensureAlwaysMsgf(IsValid(Controller),
	                      TEXT("UPlayerInteractionAbility PlayerController invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AVVM_LOGGER_LOG(LogWeaponSample,
	                Controller,
	                Controller,
	                TEXT("Input Released. Total Held Time %s."),
	                *FString::SanitizeFloat(TimeHeld, 2));

	const bool bWasCommitted = CommitAbility(Handle, ActorInfo, ActivationInfo);
	if (bWasCommitted)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UTriggerHoldAbility::OnTick(const float NewDelta)
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();

	if (CurrHeldTime >= ForceExitHeldTime)
	{
		// @gdemers Having to do this makes me question Unreal functional approach to this function signature. The Ability already has all that data
		// cached. OOP already allows access to these arguments through the Object so why require argument passing here AND more important why retrieve that data via copy (ActivationInfo and SpecHandle).
		// The functional approach here prevents actions like the one this call is made from to easily call CancelAbility without having to copy data around.
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
	else
	{
		if (!CheckCost(Handle, ActorInfo))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		else if (FMath::IsNearlyZero(CurrHeldTime))
		{
			// @gdemers first shot fired.
			Execute(Handle, ActorInfo, ActivationInfo);
		}
		else if (((CurrHeldTime / ((PredictedTriggerCount != 0) ? PredictedTriggerCount : 1)) >= TriggerProgressRate))
		{
			// @gdemers subsequent shot fired at interval.
			Execute(Handle, ActorInfo, ActivationInfo);
		}

		CurrHeldTime += NewDelta;
	}
}

void UTriggerHoldAbility::Execute(const FGameplayAbilitySpecHandle Handle,
                                  const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo)
{
	PredictedTriggerCount = FMath::Clamp(PredictedTriggerCount + 1, 1, INT_MAX);
	Super::Execute(Handle, ActorInfo, ActivationInfo);
}
