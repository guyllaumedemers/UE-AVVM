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
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "InteractionManagerComponent.h"
#include "Components/ShapeComponent.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UInteractionComponent_InstanceCounter, TEXT("Interaction Component Instance Counter"));

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UInteractionComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UInteractionComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())

	auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
	if (IsValid(CollisionComponent))
	{
		CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UInteractionComponent::OnPrimitiveComponentBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UInteractionComponent::OnPrimitiveComponentEndOverlap);
	}

	OwningOuter = Outer;
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
	if (IsValid(CollisionComponent))
	{
		CollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
		CollisionComponent->OnComponentEndOverlap.RemoveAll(this);
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UInteractionComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())
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

	const bool bIsActorLocallyControlled = UAVVMGameplayUtils::IsLocallyControlled(OtherActor);
	if (!bIsActorLocallyControlled)
	{
		return;
	}

	auto* AbilityComponent = OtherActor->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		// @gdemers allow a gameplay ability to be executable due to tag condition
		// being met!
		AbilityComponent->AddReplicatedLooseGameplayTag(GrantAbilityTag);
	}

	auto* InteractionManager = UInteractionManagerComponent::GetManager(OtherActor);
	if (IsValid(InteractionManager))
	{
		// @gdemers we try adding this local overlap event and first validate the replicated state
		// of our manager to prevent existing overlaps.
		InteractionManager->AttemptRecordBeginOverlap(OwningOuter.Get(), OtherActor, bShouldPreventContingency);
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

	const bool bIsActorLocallyControlled = UAVVMGameplayUtils::IsLocallyControlled(OtherActor);
	if (!bIsActorLocallyControlled)
	{
		return;
	}

	auto* AbilityComponent = OtherActor->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveReplicatedLooseGameplayTag(GrantAbilityTag);
	}

	auto* InteractionManager = UInteractionManagerComponent::GetManager(OtherActor);
	if (IsValid(InteractionManager))
	{
		InteractionManager->AttemptRecordEndOverlap(OwningOuter.Get(), OtherActor);
	}
}
