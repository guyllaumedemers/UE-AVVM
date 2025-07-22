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
#include "Abilities/PlayerToggleInventoryAbility.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMNotificationSubsystem.h"
#include "Data/AVVMHandshakePayload.h"
#include "GameFramework/PlayerState.h"

void UPlayerToggleInventoryAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerToggleInventoryAbility FGameplayAbilityActorInfo invalid!")))
	{
		return;
	}

	const AActor* Outer = ActorInfo->OwnerActor.Get();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Ability Granted \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *GetName(),
	       *Outer->GetName());
}

void UPlayerToggleInventoryAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerToggleInventoryAbility FGameplayAbilityActorInfo invalid!")))
	{
		return;
	}

	const AActor* Outer = ActorInfo->OwnerActor.Get();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Ability Removed \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *GetName(),
	       *Outer->GetName());
}

bool UPlayerToggleInventoryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo,
                                                       const FGameplayTagContainer* SourceTags,
                                                       const FGameplayTagContainer* TargetTags,
                                                       FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UPlayerToggleInventoryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr,
	                      TEXT("UPlayerToggleInventoryAbility FGameplayAbilityActorInfo invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const APlayerController* PC = ActorInfo->PlayerController.Get();
	if (!ensureAlwaysMsgf(IsValid(PC),
	                      TEXT("UPlayerToggleInventoryAbility PlayerController invalid!")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Attempting Ability Activation \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(PC).GetData(),
	       *GetName(),
	       *PC->GetName());

	UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
	                                        ChannelTag,
	                                        PC,
	                                        PC->PlayerState,
	                                        FAVVMNotificationPayload::Make<FAVVMHandshakePayload>(PC->PlayerState, nullptr));
}

void UPlayerToggleInventoryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               bool bReplicateEndAbility,
                                               bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
