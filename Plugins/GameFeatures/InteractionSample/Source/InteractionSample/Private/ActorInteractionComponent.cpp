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
#include "ActorInteractionComponent.h"

#include "ActorInteractionImpl.h"
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Components/ShapeComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UActorInteractionComponent_InstanceCounter, TEXT("Actor Interaction Component Instance Counter"));

UActorInteractionComponent::UActorInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UActorInteractionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorInteractionComponent, InteractionImpl);
}

void UActorInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UActorInteractionComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UActorInteractionComponent to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	if (ensureAlwaysMsgf(IsValid(InteractionImplClass),
	                     TEXT("Invalid InteractionImplClass!")))
	{
		InteractionImpl = NewObject<UActorInteractionImpl>(this, InteractionImplClass);
	}

	if (IsValid(InteractionImpl))
	{
		InteractionImpl->SafeBegin();
	}

	auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
	if (ensureAlwaysMsgf(IsValid(CollisionComponent), TEXT("Outer missing CollisionComponent!")))
	{
		CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UActorInteractionComponent::OnPrimitiveComponentBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UActorInteractionComponent::OnPrimitiveComponentEndOverlap);
	}

	OwningOuter = Outer;
}

void UActorInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsValid(InteractionImpl))
	{
		InteractionImpl->SafeEnd();
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
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
	       TEXT("Executed from \"%s\". Removing UActorInteractionComponent to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

void UActorInteractionComponent::OnPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
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

	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (IsValid(Impl))
	{
		Impl->HandleBeginOverlap(OwningOuter.Get()/*World Actor*/, OtherActor->GetInstigatorController()/*AController*/, bShouldPreventContingency);
	}
}

void UActorInteractionComponent::OnPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                                                AActor* OtherActor,
                                                                UPrimitiveComponent* OtherComp,
                                                                int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (IsValid(Impl))
	{
		Impl->HandleEndOverlap(OwningOuter.Get()/*World Actor*/, OtherActor->GetInstigatorController()/*AController*/);
	}
}
