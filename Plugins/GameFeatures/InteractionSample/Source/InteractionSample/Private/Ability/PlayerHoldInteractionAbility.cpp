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
	const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();

	if (!ensureAlwaysMsgf(ActorInfo != nullptr, TEXT("Invalid FGameplayAbilityActorInfo!")))
	{
		CancelAbility(SpecHandle, ActorInfo, ActivationInfo, true);
		return;
	}

	const AActor* Controller = ActorInfo->PlayerController.Get();
	if (!IsValid(Controller))
	{
		CancelAbility(SpecHandle, ActorInfo, ActivationInfo, true);
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
		CancelAbility(SpecHandle, ActorInfo, ActivationInfo, true);
		return;
	}

	const FInteractionExecutionFloatRequirements Requirements(TimeHeld);
	bool bCanCommit = InteractionComponent->StopExecution(Controller) && InteractionComponent->DoesMeetExecutionRequirements(
			TInstancedStruct<FInteractionExecutionFloatRequirements>::Make(Requirements));

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". \"%s\" Ability \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(GetOwningActorFromActorInfo()).GetData(),
	       bCanCommit ? TEXT("Committing") : TEXT("Aborting"),
	       *UPlayerInteractionAbilityBase::StaticClass()->GetName());

	if (bCanCommit)
	{
		CommitAbility(SpecHandle, ActorInfo, ActivationInfo);
	}
	else
	{
		CancelAbility(SpecHandle, ActorInfo, ActivationInfo, true);
	}
}

void UPlayerHoldInteractionAbility::OnTick(const float NewDelta)
{
	const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();

	if (!ensureAlwaysMsgf(ActorInfo != nullptr, TEXT("Invalid FGameplayAbilityActorInfo!")))
	{
		CancelAbility(SpecHandle, ActorInfo, ActivationInfo, true);
		return;
	}

	UActorInteractionComponent* InteractionComponent = TargetComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(InteractionComponent), TEXT("Invalid Interaction Component cached!")))
	{
		CancelAbility(SpecHandle, ActorInfo, ActivationInfo, true);
	}
	else
	{
		InteractionComponent->PumpHeartbeat(ActorInfo->PlayerController.Get(), NewDelta);
	}
}
