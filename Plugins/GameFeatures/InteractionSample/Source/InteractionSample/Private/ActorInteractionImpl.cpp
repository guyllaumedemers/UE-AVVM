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
#include "ActorInteractionImpl.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMToolkitUtils.h"
#include "InteractionObject.h"
#include "Data/AVVMHandshakePayload.h"
#include "Data/InteractionExecutionContext.h"
#include "Data/InteractionExecutionRequirements.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"

void UActorInteractionImpl::SafeBegin()
{
	const AActor* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("Binding %s."),
	                *GetNameSafe(UActorInteractionImpl::StaticClass()));

	OwningOuter = Outer;
}

void UActorInteractionImpl::SafeEnd()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("Unbinding %s."),
	                *GetNameSafe(UActorInteractionImpl::StaticClass()));

	OwningOuter.Reset();
}

bool UActorInteractionImpl::HandleBeginOverlap(const AActor* NewInstigator,
                                               const AActor* NewTarget,
                                               const bool bShouldPreventContingency,
                                               FInteractionObjectFastArray& OutRecords)
{
	if (!IsValid(NewTarget))
	{
		return false;
	}

	const bool bHasActorAuthority = UAVVMToolkitUtils::CheckActorAuthority(NewTarget);
	if (!bHasActorAuthority)
	{
		return false;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return false;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("Overlap event detected between %s, and %s."),
	                *GetNameSafe(NewInstigator),
	                *GetNameSafe(NewTarget));

	if (!bShouldPreventContingency)
	{
		return true;
	}

	bool bCanInteract = true;
	for (const FInteractionObject* Record : GetPartialMatchingInteractions(NewInstigator, OutRecords))
	{
		if ((Record != nullptr) && !Record->CanInteract())
		{
			bCanInteract = false;
			break;
		}
	}

	return bCanInteract;
}

bool UActorInteractionImpl::HandleEndOverlap(const AActor* NewInstigator,
                                             const AActor* NewTarget,
                                             FInteractionObjectFastArray& OutRecords)
{
	if (!IsValid(NewTarget))
	{
		return false;
	}

	const bool bHasActorAuthority = UAVVMToolkitUtils::CheckActorAuthority(NewTarget);
	if (!bHasActorAuthority)
	{
		return false;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return false;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("Overlap event detected between %s, and %s."),
	                *GetNameSafe(NewInstigator),
	                *GetNameSafe(NewTarget));

	const TArray<FInteractionObject*> SearchResult = GetExactMatchingInteractions(NewInstigator /*World Actor*/, NewTarget /*AController*/, OutRecords);
	return !SearchResult.IsEmpty();
}

bool UActorInteractionImpl::StartExecute(const AActor* NewInstigator,
                                         const AActor* NewTarget,
                                         const bool bShouldPreventContingency,
                                         FInteractionObjectFastArray& OutRecords)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return false;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("%s Start."),
	                *GetNameSafe(UActorInteractionImpl::StaticClass()));

#if WITH_SERVER_CODE
	if (bShouldPreventContingency && IsValid(NewTarget) && NewTarget->HasAuthority())
	{
		const bool bResult = Server_LockInteraction(NewInstigator, NewTarget, OutRecords);
		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                this,
		                TEXT("Attempt locking interaction. Result%s."),
		                bResult ? TEXT("Succeeded") : TEXT("Failed"));

		return bResult;
	}
#endif

	return true;
}

bool UActorInteractionImpl::StopExecute(const AActor* NewInstigator,
                                        const AActor* NewTarget,
                                        const bool bShouldPreventContingency,
                                        FInteractionObjectFastArray& OutRecords)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return false;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("%s Stop."),
	                *GetNameSafe(UActorInteractionImpl::StaticClass()));

#if WITH_SERVER_CODE
	if (bShouldPreventContingency && IsValid(NewTarget) && NewTarget->HasAuthority())
	{
		const bool bResult = Server_UnlockInteraction(NewInstigator, NewTarget, OutRecords);
		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                this,
		                TEXT("Attempt unlocking interaction. Result:%s."),
		                bResult ? TEXT("Succeeded") : TEXT("Failed"));

		return bResult;
	}
#endif

	return true;
}

void UActorInteractionImpl::PumpHeartbeat(const AActor* NewTarget,
                                          const float NewDelta) const
{
	const auto* Instanced = GetExecutionCtx().GetPtr<FInteractionExecutionContext>();
	if (!ensureAlwaysMsgf(Instanced != nullptr, TEXT("FInteractionExecutionContext invalid!")))
	{
		return;
	}

	Instanced->PumpHeartbeat(OwningOuter.Get(),
	                         NewTarget,
	                         NewDelta);
}

void UActorInteractionImpl::Execute(const AActor* NewTarget) const
{
	const auto* Instanced = GetExecutionCtx().GetPtr<FInteractionExecutionContext>();
	if (!ensureAlwaysMsgf(Instanced != nullptr, TEXT("FInteractionExecutionContext invalid!")))
	{
		return;
	}

	Instanced->Execute(OwningOuter.Get(),
	                   NewTarget);
}

void UActorInteractionImpl::Kill(const AActor* NewTarget) const
{
	const auto* Instanced = GetExecutionCtx().GetPtr<FInteractionExecutionContext>();
	if (!ensureAlwaysMsgf(Instanced != nullptr, TEXT("FInteractionExecutionContext invalid!")))
	{
		return;
	}

	Instanced->Kill(OwningOuter.Get(),
	                NewTarget);
}

bool UActorInteractionImpl::DoesMeetExecutionRequirements(const TInstancedStruct<FInteractionExecutionRequirements>& Compare) const
{
	const auto* Instanced = Compare.GetPtr<FInteractionExecutionRequirements>();
	return (Instanced != nullptr) ? Instanced->DoesMeetRequirements(GetRequirements()) : false;
}

const TInstancedStruct<FInteractionExecutionRequirements>& UActorInteractionImpl::GetExecutionRequirements() const
{
	return GetRequirements();
}

#if WITH_EDITOR
void UActorInteractionImpl::MoveDataToSparseClassDataStruct() const
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
	FInteractionImplSparseData* SparseClassData = GetMutableInteractionImplSparseData();

	// Modify these lines to include all Sparse Class Data properties.
	SparseClassData->GameplayEffect = GameplayEffect_DEPRECATED;
	SparseClassData->StartPromptInteractionChannel = StartPromptInteractionChannel_DEPRECATED;
	SparseClassData->StopPromptInteractionChannel = StopPromptInteractionChannel_DEPRECATED;
	SparseClassData->Requirements = Requirements_DEPRECATED;
	SparseClassData->ExecutionCtx = ExecutionCtx_DEPRECATED;
#endif // WITH_EDITORONLY_DATA
}
#endif

void UActorInteractionImpl::HandleNewRecord(const FInteractionObject& NewRecord)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("Record Collection modified. Add!"));

	const AActor* Instigator = NewRecord.GetInstigator();
	const AActor* Target = NewRecord.GetTarget();

	const auto* Controller = Cast<AController>(Target);
	if (!IsValid(Controller))
	{
		return;
	}

#if WITH_SERVER_CODE
	if (Controller->HasAuthority())
	{
		const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(GetGameplayEffect(), const_cast<AController*>(Controller), const_cast<AActor*>(Instigator));
		auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Controller->PlayerState);
		AddGameplayEffectHandle(ASC, GESpecHandle);
	}
#endif

#if WITH_EDITOR
	if (!Controller->IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        GetStartPromptInteractionChannel(),
		                                        Controller,
		                                        Instigator,
		                                        FAVVMNotificationPayload::Make<FAVVMHandshakePayload>(Instigator, Controller));
	}
}

void UActorInteractionImpl::HandlePendingKillRecord(const FInteractionObject& PendingKillRecord)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                this,
	                TEXT("Record Collection modified. Remove!"));

	if (!PendingKillRecord.IsPendingKill())
	{
		return;
	}

	const AActor* Instigator = PendingKillRecord.GetInstigator();
	const AActor* Target = PendingKillRecord.GetTarget();

	const auto* Controller = Cast<AController>(Target);
	if (!IsValid(Controller))
	{
		return;
	}

#if WITH_SERVER_CODE
	if (Controller->HasAuthority())
	{
		auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Controller->PlayerState);
		RemoveGameplayEffectHandle(ASC);
	}
#endif

#if WITH_EDITOR
	if (!Controller->IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        GetStopPromptInteractionChannel(),
		                                        Controller,
		                                        Instigator,
		                                        FAVVMNotificationPayload::Empty);
	}
}

void UActorInteractionImpl::AddGameplayEffectHandle(UAbilitySystemComponent* ASC, const FGameplayEffectSpecHandle& GEHandle)
{
	if (!IsValid(ASC))
	{
		return;
	}

	FActiveGameplayEffectHandle& SearchResult = ActorToGEActiveHandle.FindOrAdd(ASC->GetOwnerActor());
	if (SearchResult.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SearchResult);
	}

	SearchResult = ASC->BP_ApplyGameplayEffectSpecToSelf(GEHandle);
}

void UActorInteractionImpl::RemoveGameplayEffectHandle(UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC))
	{
		return;
	}

	const AActor* Instigator = ASC->GetOwnerActor();

	FActiveGameplayEffectHandle* SearchResult = ActorToGEActiveHandle.Find(Instigator);
	if (SearchResult != nullptr)
	{
		ASC->RemoveActiveGameplayEffect(*SearchResult);
		ActorToGEActiveHandle.Remove(Instigator);
	}
}

bool UActorInteractionImpl::Server_LockInteraction(const AActor* NewInstigator,
                                                   const AActor* NewTarget,
                                                   FInteractionObjectFastArray& OutRecords)
{
	bool bCanInteract = true;
	FInteractionObject* TargetInteraction = nullptr;

	for (FInteractionObject* Record : GetPartialMatchingInteractions(NewInstigator, OutRecords))
	{
		if (Record == nullptr)
		{
			continue;
		}

		if (!Record->CanInteract())
		{
			bCanInteract = false;
			break;
		}
		else if (Record->DoesExactMatch(NewInstigator, NewTarget))
		{
			TargetInteraction = Record;
		}
	}

	const bool bResult = bCanInteract && (TargetInteraction != nullptr);
	if (bResult)
	{
		OutRecords.MarkItemDirty(*TargetInteraction);
		TargetInteraction->Lock();
	}

	return bResult;
}

bool UActorInteractionImpl::Server_UnlockInteraction(const AActor* NewInstigator,
                                                     const AActor* NewTarget,
                                                     FInteractionObjectFastArray& OutRecords)
{
	TArray<FInteractionObject*> SearchResult = GetExactMatchingInteractions(NewInstigator, NewTarget, OutRecords);
	if (SearchResult.IsEmpty() || !ensureAlwaysMsgf(SearchResult.Num() == 1, TEXT("Multiple match found for unique instigator and target pair!")))
	{
		return false;
	}

	FInteractionObject* TargetInteraction = SearchResult[0];

	const bool bResult = (TargetInteraction != nullptr) && ensureAlwaysMsgf(!TargetInteraction->CanInteract(), TEXT("Target Interaction wasn't locked!"));
	if (bResult)
	{
		OutRecords.MarkItemDirty(*TargetInteraction);
		TargetInteraction->Unlock();
	}

	return bResult;
}

TArray<FInteractionObject*> UActorInteractionImpl::GetExactMatchingInteractions(const AActor* NewInstigator,
																				const AActor* NewTarget,
																				FInteractionObjectFastArray& OutRecords) const
{
	TArray<FInteractionObject*> OutResult{};
	for (auto& InteractionObject : OutRecords.InteractionObjects)
	{
		const bool bDoesMatch = InteractionObject.DoesExactMatch(NewInstigator /*World Actor*/, NewTarget /*AController*/);
		if (bDoesMatch)
		{
			OutResult.Add(&InteractionObject);
		}
	}

	return OutResult;
}

TArray<FInteractionObject*> UActorInteractionImpl::GetPartialMatchingInteractions(const AActor* NewInstigator,
																				  FInteractionObjectFastArray& OutRecords) const
{
	TArray<FInteractionObject*> OutResult{};
	for (auto& InteractionObject : OutRecords.InteractionObjects)
	{
		const bool bDoesMatch = InteractionObject.DoesPartialMatch(NewInstigator /*World Actor*/);
		if (bDoesMatch)
		{
			OutResult.Add(&InteractionObject);
		}
	}

	return OutResult;
}
