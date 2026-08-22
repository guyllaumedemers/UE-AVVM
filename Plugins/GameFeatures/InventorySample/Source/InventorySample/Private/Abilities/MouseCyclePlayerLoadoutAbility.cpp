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
#include "Abilities/MouseCyclePlayerLoadoutAbility.h"

#include "AVVMCommonInputPreprocessor.h"
#include "AVVMToolkitUtils.h"
#include "InventoryProvider.h"
#include "NonReplicatedLoadoutObject.h"
#include "Engine/Player.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void UMouseCyclePlayerLoadoutAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void UMouseCyclePlayerLoadoutAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UMouseCyclePlayerLoadoutAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FGameplayTagContainer* SourceTags,
                                                         const FGameplayTagContainer* TargetTags,
                                                         FGameplayTagContainer* OptionalRelevantTags) const
{
	const bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (!bResult)
	{
		return false;
	}

	if (!ensureAlwaysMsgf(ActorInfo != nullptr, TEXT("Invalid Actor info access.")) ||
		!ensureAlwaysMsgf(ActorInfo->PlayerController.IsValid(), TEXT("Invalid PC. Calling Outside LocalRole=AutonomousProxy, or =Authority.")))
	{
		return false;
	}

	const bool bDoesImplements = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UInventoryProvider>(ActorInfo->AvatarActor.Get());
	if (!bDoesImplements)
	{
		return false;
	}
	
	// TODO @gdemers we need to be able to track mouse wheel, its delta direction, and check if the player
	// is within menu, in-game, or in a state that prevent activation such as stasis, or death.
	const auto* LocalPlayer = Cast<ULocalPlayer>(ActorInfo->PlayerController->Player);
	const float MouseWheelDelta = UAVVMCommonInputSubsystem::Static_GetMouseWheelDelta(LocalPlayer);

	return true;
}

void UMouseCyclePlayerLoadoutAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ensureAlwaysMsgf(ActorInfo != nullptr, TEXT("Invalid Actor info access.")))
	{
		return;
	}

	auto* NonReplicatedLoadoutObject = IInventoryProvider::Execute_GetNonReplicatedLoadoutObject(ActorInfo->AvatarActor.Get());
	if (ensureAlwaysMsgf(IsValid(NonReplicatedLoadoutObject), TEXT("Inventory Provider is missing the required Loadout Object.")))
	{
		const auto* LocalPlayer = Cast<ULocalPlayer>(ActorInfo->PlayerController->Player);
		const float MouseWheelDelta = UAVVMCommonInputSubsystem::Static_GetMouseWheelDelta(LocalPlayer);
		NonReplicatedLoadoutObject->MouseCycle(MouseWheelDelta);
	}
}

void UMouseCyclePlayerLoadoutAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                                 bool bReplicateEndAbility,
                                                 bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
