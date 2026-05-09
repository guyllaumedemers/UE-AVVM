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

void FAVVMTriggeringActorAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
	
	// @gdemers TODO update anim proxy with game thread specific calculation
}

void FAVVMTriggeringActorAnimInstanceProxy::PostUpdate(UAnimInstance* InAnimInstance) const
{
	FAnimInstanceProxy::PostUpdate(InAnimInstance);
	
	// @gdemers TODO update progress calculation when coming back from worker thread.
}

UTriggeringActorAnimInstance::UTriggeringActorAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseMultiThreadedAnimationUpdate = true;
}

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
	
	// @gdemers do work here!
}

bool UTriggeringActorAnimInstance::PCV_ShouldWarnAboutNodesNotUsingFastPath() const
{
	return true;
}

void UTriggeringActorAnimInstance::OnTriggeringActorStateTagChanged(const FGameplayTagContainer& NewStateTags)
{
	auto* GameThreadProxy = GetProxyOnGameThreadStatic<FAVVMTriggeringActorAnimInstanceProxy>(this);
	if (!ensureAlwaysMsgf(GameThreadProxy != nullptr, TEXT("Invalid Anim Proxy")))
	{
		return;
	}

	UPDATE_TS_PROPERTY(GameThreadProxy->Status, bIsDraw, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_DRAW, NewStateTags);
	UPDATE_TS_PROPERTY(GameThreadProxy->Status, bIsRelaxed, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_RELAXED, NewStateTags);
	UPDATE_TS_PROPERTY(GameThreadProxy->Status, bIsRaised, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_RAISED, NewStateTags);
	UPDATE_TS_PROPERTY(GameThreadProxy->Status, bIsTriggered, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_TRIGGERED, NewStateTags);
	UPDATE_TS_PROPERTY(GameThreadProxy->Status, bIsExhausted, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_EXHAUSTED, NewStateTags);
	UPDATE_TS_PROPERTY(GameThreadProxy->Status, bIsRecharging, TAG_WEAPONSAMPLE_TRIGGERING_ACTOR_STATUS_STATE_RECHARGING, NewStateTags);
}

FAnimInstanceProxy* UTriggeringActorAnimInstance::CreateAnimInstanceProxy()
{
	return &AnyThreadProxy;
}

void UTriggeringActorAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	// @gdemers our proxy is not heap allocated.
	// Super::DestroyAnimInstanceProxy(InProxy);
}
