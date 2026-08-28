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
#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "AVVMReplicatedTagComponent.h"
#include "AVVMTagUtils.h"
#include "InteractionManagerSubsystem.h"
#include "InteractionObject.h"
#include "Components/ShapeComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UActorInteractionComponent_InstanceCounter, TEXT("Actor Interaction Component Instance Counter"));

UActorInteractionComponent::UActorInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = false;
}

void UActorInteractionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UActorInteractionComponent, Records, Params);
}

void UActorInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// @gdemers allow control over collection size based on user-defined requirements.
	Records.InteractionObjects.Reset(GetDefaultAllocationSize());

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	OwningOuter = Outer;

	TRACE_COUNTER_INCREMENT(UActorInteractionComponent_InstanceCounter);
	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(UActorInteractionComponent::StaticClass()));

	if (ensureAlwaysMsgf(IsValid(GetInteractionImplClass()),
	                     TEXT("Invalid InteractionImplClass!")))
	{
		InteractionImpl = NewObject<UActorInteractionImpl>(this, GetInteractionImplClass());
		InteractionImpl->SafeBegin();
	}

#if WITH_SERVER_CODE
	if (!Outer->HasAuthority())
	{
		return;
	}

	Handle = UInteractionManagerSubsystem::Static_Register(GetWorld(), this);

	auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
	if (ensureAlwaysMsgf(IsValid(CollisionComponent), TEXT("Outer missing CollisionComponent!")))
	{
		CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UActorInteractionComponent::OnPrimitiveComponentBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UActorInteractionComponent::OnPrimitiveComponentEndOverlap);
	}
#endif
}

void UActorInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Records.InteractionObjects.Empty();
	if (IsValid(InteractionImpl))
	{
		InteractionImpl->SafeEnd();
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UActorInteractionComponent::StaticClass()));

	OwningOuter.Reset();

#if WITH_SERVER_CODE
	if (!Outer->HasAuthority())
	{
		return;
	}

	UInteractionManagerSubsystem::Static_Unregister(GetWorld(), Handle);

	auto* CollisionComponent = Outer->GetComponentByClass<UShapeComponent>();
	if (IsValid(CollisionComponent))
	{
		CollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
		CollisionComponent->OnComponentEndOverlap.RemoveAll(this);
	}
#endif
}

UActorInteractionComponent* UActorInteractionComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UActorInteractionComponent>() : nullptr;
}

bool UActorInteractionComponent::StartExecution(const AActor* NewTarget)
{
	return IsValid(InteractionImpl)
		       ? InteractionImpl->StartExecute(OwningOuter.Get(), NewTarget, GetInteractionSparseData(EGetSparseClassDataMethod::ArchetypeIfNull)->bShouldPreventContingency, Records)
		       : false;
}

bool UActorInteractionComponent::StopExecution(const AActor* NewTarget)
{
	return IsValid(InteractionImpl)
		       ? InteractionImpl->StopExecute(OwningOuter.Get(), NewTarget, GetInteractionSparseData(EGetSparseClassDataMethod::ArchetypeIfNull)->bShouldPreventContingency, Records)
		       : false;
}

bool UActorInteractionComponent::DoesMeetExecutionRequirements(const TInstancedStruct<FInteractionExecutionRequirements>& Compare) const
{
	return IsValid(InteractionImpl)
		       ? InteractionImpl->DoesMeetExecutionRequirements(Compare)
		       : false;
}

void UActorInteractionComponent::GetInteractionRequirements(TInstancedStruct<FInteractionExecutionRequirements>& OutRequirements) const
{
	if (IsValid(InteractionImpl))
	{
		OutRequirements = InteractionImpl->GetExecutionRequirements();
	}
}

void UActorInteractionComponent::PumpHeartbeat(const AActor* NewTarget, const float NewDelta) const
{
	if (IsValid(InteractionImpl))
	{
		InteractionImpl->PumpHeartbeat(NewTarget, NewDelta);
	}
}

void UActorInteractionComponent::Execute(const AActor* NewTarget) const
{
	if (IsValid(InteractionImpl))
	{
		InteractionImpl->Execute(NewTarget);
	}
}

void UActorInteractionComponent::Kill(const AActor* NewTarget) const
{
	if (IsValid(InteractionImpl))
	{
		InteractionImpl->Kill(NewTarget);
	}
}

#if WITH_EDITOR
void UActorInteractionComponent::MoveDataToSparseClassDataStruct() const
{
	// make sure we don't overwrite the sparse data if it has been saved already
	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass == nullptr || BPClass->bIsSparseClassDataSerializable == true)
	{
		return;
	}
 
	Super::MoveDataToSparseClassDataStruct();

#if WITH_EDITORONLY_DATA
	// Unreal Header Tool (UHT) will create GetMySparseClassData automatically.
	FInteractionSparseData* SparseClassData = GetMutableInteractionSparseData();

	// Modify these lines to include all Sparse Class Data properties.
	SparseClassData->RequiredTags = RequiredTags_DEPRECATED;
	SparseClassData->BlockingTags = BlockingTags_DEPRECATED;
	SparseClassData->bShouldPreventContingency = bShouldPreventContingency_DEPRECATED;
	SparseClassData->DefaultAllocationSize = DefaultAllocationSize_DEPRECATED;
	SparseClassData->InteractionImplClass = InteractionImplClass_DEPRECATED;
#endif // WITH_EDITORONLY_DATA
}
#endif

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

	const AController* Target = OtherActor->GetInstigatorController();
	const AActor* Instigator = OwningOuter.Get();

	const UAVVMReplicatedTagComponent* ReplicatedTagComponent = nullptr;
	if (IsValid(Target))
	{
		const AActor* PlayerState = IsValid(Target) ? Target->PlayerState : nullptr;
		ReplicatedTagComponent = UAVVMReplicatedTagComponent::GetActorComponent(IsValid(PlayerState) ? PlayerState : Target->GetPawn());
		ensureAlwaysMsgf(IsValid(ReplicatedTagComponent), TEXT("Attempt to retrieve %s from invalid target."), *GetNameSafe(UAVVMReplicatedTagComponent::StaticClass()));
	}

	if (!UAVVMTagUtils::DoesMeetRequirements(ReplicatedTagComponent, GetRequiredTags(), GetBlockingTags()))
	{
		return;
	}
	
	Server_ClearPendingKill();
	
	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (!IsValid(Impl))
	{
		return;
	}
	
	const bool bResult = Impl->HandleBeginOverlap(Instigator/*World Actor*/,
	                                              Target/*AController*/,
	                                              GetInteractionSparseData(EGetSparseClassDataMethod::ArchetypeIfNull)->bShouldPreventContingency,
	                                              Records);
	
	if (bResult)
	{
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

	const AActor* Instigator = OwningOuter.Get();
	const AController* Target = OtherActor->GetInstigatorController();

	const UAVVMReplicatedTagComponent* ReplicatedTagComponent = nullptr;
	if (IsValid(Target))
	{
		const AActor* PlayerState = IsValid(Target) ? Target->PlayerState : nullptr;
		ReplicatedTagComponent = UAVVMReplicatedTagComponent::GetActorComponent(IsValid(PlayerState) ? PlayerState : Target->GetPawn());
		ensureAlwaysMsgf(IsValid(ReplicatedTagComponent), TEXT("Attempt to retrieve %s from invalid target."), *GetNameSafe(UAVVMReplicatedTagComponent::StaticClass()));
	}

	if (!UAVVMTagUtils::DoesMeetRequirements(ReplicatedTagComponent, GetRequiredTags(), GetBlockingTags()))
	{
		return;
	}

	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (!IsValid(Impl))
	{
		return;
	}

	const bool bResult = Impl->HandleEndOverlap(Instigator/*World Actor*/,
	                                            Target/*AController*/,
	                                            Records);

	if (bResult)
	{
		Server_SetPendingKill(Instigator, Target);
	}
}

void UActorInteractionComponent::Server_AddRecord(const AActor* NewInstigator,
                                                  const AActor* NewTarget)
{
	Records.MarkArrayDirty();
	Records.InteractionObjects.Add(FInteractionObject{NewTarget, NewInstigator, this});
	HandleNewRecord(Records.InteractionObjects.Top());
}

void UActorInteractionComponent::Server_SetPendingKill(const AActor* NewInstigator,
                                                       const AActor* NewTarget)
{
	FInteractionObject* SearchResult = Records.InteractionObjects.FindByPredicate([&](const FInteractionObject& Param)
	{
		return Param.DoesExactMatch(NewInstigator /*World Actor*/, NewTarget /*AController*/);
	});

	if (SearchResult != nullptr)
	{
		Records.MarkItemDirty(*SearchResult);
		SearchResult->SetPendingKill();

		HandlePendingKillRecord(*SearchResult);
	}
}

void UActorInteractionComponent::Server_ClearPendingKill()
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!Outer->HasAuthority() || Records.InteractionObjects.IsEmpty())
	{
		return;
	}

	Records.MarkArrayDirty();
	for (int32 i = Records.InteractionObjects.Num() - 1; i >= 0; --i)
	{
		const FInteractionObject& Record = Records.InteractionObjects[i];
		if (!Record.IsPendingKill())
		{
			continue;
		}

		Records.InteractionObjects.RemoveSingleSwap(Record);
	}
}

void UActorInteractionComponent::HandleNewRecord(const FInteractionObject& NewRecord)
{
	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (IsValid(Impl))
	{
		Impl->HandleNewRecord(NewRecord);
	}
}

void UActorInteractionComponent::HandlePendingKillRecord(const FInteractionObject& PendingKillRecord)
{
	UActorInteractionImpl* Impl = InteractionImpl.Get();
	if (IsValid(Impl))
	{
		Impl->HandlePendingKillRecord(PendingKillRecord);
	}
}
