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
#include "DynamicHitBoxComponent.h"

#include "AVVMCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDynamicHitboxComponent::UDynamicHitboxComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bAllowTickBatching = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	SetIsReplicatedByDefault(true);
}

void UDynamicHitboxComponent::BeginPlay()
{
	Super::BeginPlay();

	auto* Character = GetTypedOuter<AAVVMCharacter>();
	if (!IsValid(Character))
	{
		return;
	}

	MovementComponent = Character->GetMovementComponent();
	SkeletalMeshComponent = Character->GetMesh();
	OuterCharacter = Character;
}

void UDynamicHitboxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	MovementComponent.Reset();
	SkeletalMeshComponent.Reset();
	OuterCharacter.Reset();
}

void UDynamicHitboxComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FGameplayTag MovementTag = GetOuterMovementTag();
	if (!MovementTag.IsValid())
	{
		return;
	}

	const EPhysicState NewState = GetPhysicState(MovementTag);
	if (PreviousState != NewState)
	{
		PreviousState = NewState;
		DeferredPhysicSwap();
	}
}

FGameplayTag UDynamicHitboxComponent::GetOuterMovementTag() const
{
	// TODO @gdemers define where the movement tags must live. prob Movement component
	// but what if we use the Mover plugin instead ?
	return FGameplayTag::EmptyTag;
}

EPhysicState UDynamicHitboxComponent::GetPhysicState(const FGameplayTag& MovementTag) const
{
	if (MovementTagToPhysicAssets.Contains(MovementTag))
	{
		return MovementTagToPhysicAssets[MovementTag];
	}
	else
	{
		return EPhysicState::Default;
	}
}

USkeletalMeshComponent* UDynamicHitboxComponent::GetSkeletalMeshComponent()
{
	USkeletalMeshComponent* NewSkeletalMeshComponent = SkeletalMeshComponent.Get();
	if (IsValid(NewSkeletalMeshComponent))
	{
		return NewSkeletalMeshComponent;
	}

	const AAVVMCharacter* Character = OuterCharacter.Get();
	if (!IsValid(Character))
	{
		return nullptr;
	}

	SkeletalMeshComponent = Character->GetMesh();
	return SkeletalMeshComponent.Get();
}

void UDynamicHitboxComponent::DeferredPhysicSwap()
{
	if (!ensureAlwaysMsgf(!PhysicAssets.IsEmpty(), TEXT("Physics Assets are Empty.")))
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const auto OnNextTick = [](const TWeakObjectPtr<USkeletalMeshComponent>& NewSkeletalMeshComponent,
	                           const EPhysicState* ActivePhysicState,
	                           const TMap<EPhysicState, TObjectPtr<UPhysicsAsset>>& NewPhysicAssets)
	{
		if (ActivePhysicState == nullptr || !NewSkeletalMeshComponent.IsValid())
		{
			return;
		}

		const bool bHasFound = NewPhysicAssets.Contains(*ActivePhysicState);
		if (bHasFound)
		{
			NewSkeletalMeshComponent->SetPhysicsAsset(NewPhysicAssets[*ActivePhysicState], true);
		}
	};

	const auto Callback = FTimerDelegate::CreateWeakLambda(this, OnNextTick, GetSkeletalMeshComponent(), &PreviousState, PhysicAssets);
	World->GetTimerManager().SetTimerForNextTick(Callback);
}
