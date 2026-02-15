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
#include "Animations/TriggeringActorAnimInstance.h"

#include "AVVMReplicatedTagComponent.h"
#include "NativeGameplayTags.h"
#include "TriggeringActor.h"
#include "Animations/AVVMTSAnimInstanceUtils.h"
#include "Tags/PrivateTags.h"

void UTriggeringActorAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	OwningActor = GetTypedOuter<ATriggeringActor>();
}

void UTriggeringActorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	const ATriggeringActor* TriggeringActor = OwningActor.Get();
	if (!IsValid(TriggeringActor))
	{
		return;
	}
	
	// @gdemers replicated tags should handle tracking triggering actor state across clients
	// and simulate headless animations.
	auto* ReplicatedTagComponent = TriggeringActor->GetComponentByClass<UAVVMReplicatedTagComponent>();
	if (IsValid(ReplicatedTagComponent))
	{
		ReplicatedTagComponent->OnReplicatedTagChanged.AddUniqueDynamic(this, &UTriggeringActorAnimInstance::OnTriggeringActorStateTagChanged);
	}
}

void UTriggeringActorAnimInstance::NativeUninitializeAnimation()
{
	Super::NativeUninitializeAnimation();
	
	const ATriggeringActor* TriggeringActor = OwningActor.Get();
	if (!IsValid(TriggeringActor))
	{
		return;
	}

	auto* ReplicatedTagComponent = TriggeringActor->GetComponentByClass<UAVVMReplicatedTagComponent>();
	if (IsValid(ReplicatedTagComponent))
	{
		ReplicatedTagComponent->OnReplicatedTagChanged.RemoveAll(this);
	}
}

void UTriggeringActorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UTriggeringActorAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

void UTriggeringActorAnimInstance::OnTriggeringActorStateTagChanged(const FGameplayTagContainer& NewStateTags)
{
	// @gdemers status
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsDraw, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_DRAW, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsRelaxed, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_RELAXED, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsRaised, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_RAISED, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsTriggered, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_TRIGGERED, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsExhausted, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_EXHAUSTED, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsRecharging, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_RECHARGING, NewStateTags);
}