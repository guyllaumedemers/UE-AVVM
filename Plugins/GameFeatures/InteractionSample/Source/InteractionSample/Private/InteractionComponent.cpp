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
#include "InteractionComponent.h"

#include "AbilitySystemComponent.h"
#include "InteractionManagerComponent.h"
#include "Components/PrimitiveComponent.h"

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Actor = GetTypedOuter<AActor>();
	if (!IsValid(Actor))
	{
		return;
	}

	auto* CollisionComponent = Actor->GetComponentByClass<UPrimitiveComponent>();
	if (IsValid(CollisionComponent))
	{
		CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UInteractionComponent::OnPrimitiveComponentBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UInteractionComponent::OnPrimitiveComponentEndOverlap);
	}

	WorldActor = Actor;
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Actor = WorldActor.Get();
	if (!IsValid(Actor))
	{
		return;
	}

	auto* CollisionComponent = Actor->GetComponentByClass<UPrimitiveComponent>();
	if (IsValid(CollisionComponent))
	{
		CollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
		CollisionComponent->OnComponentEndOverlap.RemoveAll(this);
	}
}

void UInteractionComponent::OnPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                             AActor* OtherActor,
                                                             UPrimitiveComponent* OtherComp,
                                                             int32 OtherBodyIndex,
                                                             bool bFromSweep,
                                                             const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	const bool bIsActorLocallyControlled = (OtherActor->GetLocalRole() == ROLE_AutonomousProxy); /*Controlled by Client*/
	if (!bIsActorLocallyControlled)
	{
		return;
	}

	auto* AbilityComponent = OtherActor->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		// @gdemers allow a gameplay ability to be executable due to tag condition
		// being met!
		AbilityComponent->AddLooseGameplayTag(GrantAbilityTag);
	}

	auto* InteractionManager = UInteractionManagerComponent::GetManager(this);
	if (IsValid(InteractionManager))
	{
		// @gdemers we try adding this local overlap event and first validate the replicated state
		// of our manager to prevent existing overlaps.
		InteractionManager->AttemptRecordBeginOverlap(WorldActor.Get(), OtherActor, bShouldPreventContingency);
	}
}

void UInteractionComponent::OnPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                                           AActor* OtherActor,
                                                           UPrimitiveComponent* OtherComp,
                                                           int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	const bool bIsActorLocallyControlled = (OtherActor->GetLocalRole() == ROLE_AutonomousProxy); /*Controlled by Client*/
	if (!bIsActorLocallyControlled)
	{
		return;
	}

	auto* AbilityComponent = OtherActor->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveLooseGameplayTag(GrantAbilityTag);
	}

	auto* InteractionManager = UInteractionManagerComponent::GetManager(this);
	if (IsValid(InteractionManager))
	{
		InteractionManager->AttemptRecordEndOverlap(WorldActor.Get(), OtherActor);
	}
}
