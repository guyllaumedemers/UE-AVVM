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
#include "Animations/AVVMCharacterAnimInstance.h"

#include "AVVMCharacter.h"
#include "AVVMReplicatedTagComponent.h"
#include "Animations/AVVMTSAnimInstanceUtils.h"
#include "Tags/AVVMGameplayTags.h"

void UAVVMCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	OwningCharacter = GetTypedOuter<AAVVMCharacter>();
}

void UAVVMCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	const AAVVMCharacter* Character = OwningCharacter.Get();
	if (!IsValid(Character))
	{
		return;
	}
	
	// @gdemers replicated tags should handle tracking player state across clients
	// and simulate headless animations.
	auto* ReplicatedTagComponent = Character->GetComponentByClass<UAVVMReplicatedTagComponent>();
	if (IsValid(ReplicatedTagComponent))
	{
		ReplicatedTagComponent->OnReplicatedTagChanged.AddUniqueDynamic(this, &UAVVMCharacterAnimInstance::OnCharacterStateTagChanged);
	}
}

void UAVVMCharacterAnimInstance::NativeUninitializeAnimation()
{
	Super::NativeUninitializeAnimation();
	
	const AAVVMCharacter* Character = OwningCharacter.Get();
	if (!IsValid(Character))
	{
		return;
	}

	auto* ReplicatedTagComponent = Character->GetComponentByClass<UAVVMReplicatedTagComponent>();
	if (IsValid(ReplicatedTagComponent))
	{
		ReplicatedTagComponent->OnReplicatedTagChanged.RemoveAll(this);
	}
}

void UAVVMCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAVVMCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

void UAVVMCharacterAnimInstance::OnCharacterStateTagChanged(const FGameplayTagContainer& NewStateTags)
{
	// @gdemers movements
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsIdle, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_IDLE, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsWalking, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_WALK, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsSprinting, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_SPRINT, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsRunning, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_RUN, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsSliding, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_SLIDING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsCharging, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_CHARGING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsSwimming, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_SWIMMING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsGliding, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_GLIDING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsMantling, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_MANTLING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsVaulting, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_VAULTING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsProne, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_PRONE, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsCrouching, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_CROUCHING, NewStateTags);
	UPDATE_TS_PROPERTY(MovementProperties_TS, bIsStanding, TAG_AVVMGAMEPLAY_PLAYER_MOVEMENT_STATE_STANDING, NewStateTags);
	
	// @gdemers status
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsGrounded, TAG_AVVMGAMEPLAY_PLAYER_STATUS_STATE_GROUND, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsAired, TAG_AVVMGAMEPLAY_PLAYER_STATUS_STATE_AIR, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsDown, TAG_AVVMGAMEPLAY_PLAYER_STATUS_STATE_DOWN, NewStateTags);
	UPDATE_TS_PROPERTY(StatusProperties_TS, bIsRagdoll, TAG_AVVMGAMEPLAY_PLAYER_STATUS_STATE_RAGDOLL, NewStateTags);
}
