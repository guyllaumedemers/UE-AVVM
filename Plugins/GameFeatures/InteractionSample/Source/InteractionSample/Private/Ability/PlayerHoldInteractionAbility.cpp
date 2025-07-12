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
#include "Ability/PlayerHoldInteractionAbility.h"

#include "ActorInteractionComponent.h"
#include "ActorInteractionImpl.h"
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Ability/AVVMAbilityTask_TickUntil.h"
#include "Data/InteractionExecutionRequirements.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

void UPlayerHoldInteractionAbility::RunOptionalTask(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	PlayerController = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;

	auto* ParentTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (IsValid(ParentTask))
	{
		ParentTask->OnRelease.AddUniqueDynamic(this, &UPlayerHoldInteractionAbility::OnInputReleased);
		ParentTask->ReadyForActivation();
	}

	const APlayerController* PC = (ActorInfo != nullptr) ? ActorInfo->PlayerController.Get() : nullptr;
	if (!IsValid(PC) || !PC->IsLocalController())
	{
		return;
	}

	auto* ChildTask = UAVVMAbilityTask_TickUntil::TickUntil(this, true);
	if (IsValid(ChildTask))
	{
		ChildTask->OnTick.AddUniqueDynamic(this, &UPlayerHoldInteractionAbility::OnTick);
		ChildTask->ReadyForActivation();
	}
}

void UPlayerHoldInteractionAbility::OnInputReleased(float TimeHeld)
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

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Input Released on Outer \"%s\". Total Held Time \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Controller).GetData(),
	       *Controller->GetName(),
	       *FString::SanitizeFloat(TimeHeld, 2));

	UActorInteractionComponent* InteractionComponent = TargetComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(InteractionComponent), TEXT("Invalid Interaction Component cached!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const auto Requirements = FInteractionExecutionRequirements::Make<FInteractionExecutionFloatRequirements>(TimeHeld);
	bool bCanCommit = InteractionComponent->StopExecution(Controller) && InteractionComponent->DoesMeetExecutionRequirements(Requirements);

	if (bCanCommit)
	{
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
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UPlayerHoldInteractionAbility::OnTick(const float NewDelta)
{
	UActorInteractionComponent* InteractionComponent = TargetComponent.Get();
	if (ensureAlwaysMsgf(IsValid(InteractionComponent), TEXT("Invalid Interaction Component cached!")))
	{
		InteractionComponent->PumpHeartbeat(PlayerController.Get(), NewDelta);
	}
	else
	{
		// @gdemers Having to do this makes me question Unreal functional approach to this function signature. The Ability already has all that data
		// cached. OOP already allows access to these arguments through the Object so why require argument passing here AND more important why retrieve that data via copy (ActivationInfo and SpecHandle).
		// The functional approach here prevents actions like the one this call is made from to easily call CancelAbility without having to copy data around.
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
}
