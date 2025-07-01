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
#include "Interaction.h"
#include "Ability/InteractionExecutionContext.h"
#include "Ability/InteractionExecutionRequirements.h"
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

	DOREPLIFETIME(UActorInteractionComponent, Records);
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
	       TEXT("Executed from \"%s\". Adding \"%s\" Class Instance to Outer \"%s\"."),
	       *UActorInteractionComponent::StaticClass()->GetName(),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	if (ensureAlwaysMsgf(IsValid(InteractionImplClass),
	                     TEXT("Invalid InteractionImplClass!")))
	{
		InteractionImpl = NewObject<UActorInteractionImpl>(this, InteractionImplClass);
		InteractionImpl->SafeBegin();
	}

#if WITH_SERVER_CODE
	if (GetOwnerRole() == ROLE_Authority)
	{
		auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
		if (ensureAlwaysMsgf(IsValid(CollisionComponent), TEXT("Outer missing CollisionComponent!")))
		{
			CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UActorInteractionComponent::OnPrimitiveComponentBeginOverlap);
			CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UActorInteractionComponent::OnPrimitiveComponentEndOverlap);
		}
	}
#endif

	OwningOuter = Outer;
}

void UActorInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto Iterator = Records.CreateIterator(); Iterator; ++Iterator)
	{
		if (IsValid(*Iterator))
		{
			RemoveReplicatedSubObject(Iterator->Get());
			Iterator.RemoveCurrentSwap();
		}
	}

	if (IsValid(InteractionImpl))
	{
		InteractionImpl->SafeEnd();
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

#if WITH_SERVER_CODE
	if (GetOwnerRole() == ROLE_Authority)
	{
		auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
		if (IsValid(CollisionComponent))
		{
			CollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
			CollisionComponent->OnComponentEndOverlap.RemoveAll(this);
		}
	}
#endif

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" Class Instance to Outer \"%s\"."),
	       *UActorInteractionComponent::StaticClass()->GetName(),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

bool UActorInteractionComponent::StartExecution(const AActor* NewTarget) const
{
	return IsValid(InteractionImpl)
		       ? InteractionImpl->StartExecute(OwningOuter.Get(), NewTarget, Records, bShouldPreventContingency)
		       : false;
}

bool UActorInteractionComponent::StopExecution(const AActor* NewTarget) const
{
	return IsValid(InteractionImpl)
		       ? InteractionImpl->StopExecute(OwningOuter.Get(), NewTarget, Records, bShouldPreventContingency)
		       : false;
}

bool UActorInteractionComponent::DoesMeetExecutionRequirements(const TInstancedStruct<FInteractionExecutionRequirements>& Requirements) const
{
	const auto* Instanced = Requirements.GetPtr<FInteractionExecutionRequirements>();
	return (Instanced != nullptr) ? Instanced->DoesMeetRequirements(InteractionImpl) : false;
}

void UActorInteractionComponent::Execute(const AActor* NewTarget) const
{
	const auto* Instanced = ExecutionCtx.GetPtr<FInteractionExecutionContext>();
	if (ensureAlwaysMsgf(Instanced != nullptr, TEXT("FInteractionExecutionContext invalid!")))
	{
		Instanced->Execute(OwningOuter.Get(), NewTarget);
	}
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
	if (!IsValid(Impl))
	{
		return;
	}

	const AActor* Instigator = OwningOuter.Get();
	const AActor* Target = OtherActor->GetInstigatorController();

	const bool bResult = Impl->HandleBeginOverlap(Records,
	                                              Instigator/*World Actor*/,
	                                              Target/*AController*/,
	                                              bShouldPreventContingency);

	if (bResult)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Attempt Executing Server_AddRecord..."),
		       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData());

		Server_AddRecord(Instigator, Target);
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
	if (!IsValid(Impl))
	{
		return;
	}

	const AActor* Instigator = OwningOuter.Get();
	const AActor* Target = OtherActor->GetInstigatorController();

	const bool bResult = Impl->HandleEndOverlap(Records,
	                                            Instigator/*World Actor*/,
	                                            Target/*AController*/);

	if (bResult)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Attempt Executing ServerRPC_RemoveRecord..."),
		       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData());

		Server_RemoveRecord(Instigator, Target);
	}
}

void UActorInteractionComponent::Server_AddRecord(const AActor* NewInstigator,
                                                  const AActor* NewTarget)
{
	TArray<UInteraction*> OldRecords = Records;

	auto* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewInstigator /*World Actor*/, NewTarget /*AController*/);
	AddReplicatedSubObject(Transaction);
	Records.Add(Transaction);

	OnRep_RecordModified(OldRecords);
}

void UActorInteractionComponent::Server_RemoveRecord(const AActor* NewInstigator,
                                                     const AActor* NewTarget)
{
	const TObjectPtr<UInteraction>* SearchResult = Records.FindByPredicate([&](const UInteraction* Param)
	{
		return IsValid(Param) && Param->DoesExactMatch(NewInstigator /*World Actor*/, NewTarget /*AController*/);
	});

	if (SearchResult != nullptr)
	{
		TArray<UInteraction*> OldRecords = Records;

		UInteraction* Transaction = SearchResult->Get();
		RemoveReplicatedSubObject(Transaction);
		Records.Remove(Transaction);

		OnRep_RecordModified(OldRecords);
	}
}

void UActorInteractionComponent::OnRep_RecordModified(TArray<UInteraction*> OldRecords)
{
	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (IsValid(Impl))
	{
		Impl->HandleRecordModified(OldRecords, Records);
	}
}
