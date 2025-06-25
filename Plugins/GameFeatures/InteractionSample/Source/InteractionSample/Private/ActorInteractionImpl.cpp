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
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMNotificationSubsystem.h"
#include "Interaction.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void UActorInteractionImpl::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorInteractionImpl, Records);
}

bool UActorInteractionImpl::IsSupportedForNetworking() const
{
	return true;
}

#if UE_WITH_IRIS
void UActorInteractionImpl::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicaitonFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UActorInteractionImpl::SafeBegin()
{
	const AActor* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Creating \"%s\" Class Instance on Outer \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorInteractionImpl::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter = Outer;
}

void UActorInteractionImpl::SafeEnd()
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Destroying \"%s\" Class Instance on Outer \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorInteractionImpl::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter.Reset();
}

void UActorInteractionImpl::HandleBeginOverlap(const AActor* NewInstigator,
                                               const AActor* NewTarget,
                                               const bool bShouldPreventContingency)
{
	if (!IsValid(NewTarget))
	{
		return;
	}

	const bool bHasActorAuthority = UAVVMGameplayUtils::CheckActorAuthority(NewTarget);
	if (!bHasActorAuthority)
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Begin Overlap detected between Instigator \"%s\" and Target \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *NewInstigator->GetName(),
	       *NewTarget->GetName());

	const bool bResult = AttemptBeginOverlap(NewInstigator /*World Actor*/, bShouldPreventContingency);
	if (bResult)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Attempt Executing Server_AddRecord..."),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData());

		ServerRPC_AddRecord(NewInstigator /*World Actor*/, NewTarget /*AController*/);
	}
}

void UActorInteractionImpl::HandleEndOverlap(const AActor* NewInstigator,
                                             const AActor* NewTarget)
{
	if (!IsValid(NewTarget))
	{
		return;
	}

	const bool bHasActorAuthority = UAVVMGameplayUtils::CheckActorAuthority(NewTarget);
	if (!bHasActorAuthority)
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". End Overlap detected between Instigator \"%s\" and Target \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *NewInstigator->GetName(),
	       *NewTarget->GetName());

	const bool bResult = AttemptEndOverlap(NewInstigator /*World Actor*/, NewTarget /*AController*/);
	if (bResult)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Attempt Executing ServerRPC_RemoveRecord..."),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData());

		ServerRPC_RemoveRecord(NewInstigator /*World Actor*/, NewTarget /*AController*/);
	}
}

TArray<UInteraction*> UActorInteractionImpl::GetExactMatchingInteractions(const AActor* NewInstigator,
                                                                          const AActor* NewTarget) const
{
	return Records.FilterByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesExactMatch(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	});
}

TArray<UInteraction*> UActorInteractionImpl::GetPartialMatchingInteractions(const AActor* NewInstigator) const
{
	return Records.FilterByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesPartialMatch(NewInstigator);
	});
}

bool UActorInteractionImpl::AttemptBeginOverlap(const AActor* NewInstigator, const bool bShouldPreventContingency)
{
	if (!bShouldPreventContingency)
	{
		return true;
	}

	bool bCanInteract = true;
	for (const UInteraction* Interaction : GetPartialMatchingInteractions(NewInstigator))
	{
		if (IsValid(Interaction) && !Interaction->CanInteract())
		{
			bCanInteract = false;
			break;
		}
	}

	const bool bExecute = (bShouldPreventContingency && bCanInteract);
	return bExecute;
}

bool UActorInteractionImpl::AttemptEndOverlap(const AActor* NewInstigator,
                                              const AActor* NewTarget)
{
	const auto MatchingInteractions = GetExactMatchingInteractions(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	return !MatchingInteractions.IsEmpty();
}

void UActorInteractionImpl::ServerRPC_AddRecord_Implementation(const AActor* NewInstigator,
                                                               const AActor* NewTarget)
{
	TArray<UInteraction*> OldRecords = Records;

	auto* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewInstigator /*World Actor*/, NewTarget /*AController*/);

	Records.Add(Transaction);

	OnRep_RecordModified(OldRecords);
}

void UActorInteractionImpl::ServerRPC_RemoveRecord_Implementation(const AActor* NewInstigator,
                                                                  const AActor* NewTarget)
{
	const TObjectPtr<UInteraction>* SearchResult = Records.FindByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesExactMatch(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	});

	if (SearchResult != nullptr)
	{
		TArray<UInteraction*> OldRecords = Records;

		UInteraction* Transaction = SearchResult->Get();
		Records.Remove(Transaction);

		OnRep_RecordModified(OldRecords);
	}
}

void UActorInteractionImpl::OnRep_RecordModified(TArray<UInteraction*> OldRecords)
{
	const int32 OldNum = OldRecords.Num();
	const int32 NewNum = Records.Num();

	if (OldNum < NewNum)
	{
		HandleNewRecord(Records);
	}
	else if (OldNum > NewNum)
	{
		HandleOldRecord(OldRecords);
	}
}

void UActorInteractionImpl::HandleNewRecord(const TArray<UInteraction*>& NewRecords)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Record Collection modified on Outer \"%s\". Adding!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	if (NewRecords.IsEmpty())
	{
		return;
	}

	const UInteraction* TopRecord = NewRecords.Top();
	if (!IsValid(TopRecord))
	{
		return;
	}

	const AActor* Instigator = TopRecord->GetInstigator();
	const AActor* Target = TopRecord->GetTarget();

	auto* PC = Cast<AController>(Target);
	if (!IsValid(PC))
	{
		return;
	}

	APlayerState* PlayerState = PC->PlayerState;

#if WITH_SERVER_CODE
	auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
	const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(GameplayEffect, PlayerState /*APlayerState*/, const_cast<AActor*>(Instigator)/*World Actor*/);
	AddGameplayEffectHandle(ASC, GESpecHandle);
#endif

	UE_AVVM_NOTIFY_IF_LOCALLYCONTROLLED(this,
	                                    StartPromptInteractionChannel,
	                                    PC,
	                                    Instigator,
	                                    FAVVMNotificationPayload::Empty);
}

void UActorInteractionImpl::HandleOldRecord(const TArray<UInteraction*>& OldRecords)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Record Collection modified on Outer \"%s\". Removing!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	if (OldRecords.IsEmpty())
	{
		return;
	}

	const UInteraction* FoundMatch = nullptr;
	for (const TObjectPtr<UInteraction>& OldRecord : OldRecords)
	{
		if (!IsValid(OldRecord))
		{
			continue;
		}

		const TObjectPtr<UInteraction>* SearchResult = Records.FindByPredicate([&](const TObjectPtr<UInteraction>& Param)
		{
			return IsValid(Param) && Param->DoesExactMatch(OldRecord->GetInstigator() /*World Actor*/, OldRecord->GetTarget() /*APlayerCharacter*/);
		});

		if (SearchResult == nullptr)
		{
			// @gdemers rip. no longer with us!
			FoundMatch = OldRecord;
			break;
		}
	}

	if (!ensureAlwaysMsgf(IsValid(FoundMatch),
	                      TEXT("HandleOldRecord didnt find a match for removal!")))
	{
		return;
	}

	const AActor* Instigator = FoundMatch->GetInstigator();
	const AActor* Target = FoundMatch->GetTarget();

	auto* PC = Cast<AController>(Target);
	if (!IsValid(PC))
	{
		return;
	}

	APlayerState* PlayerState = PC->PlayerState;

#if WITH_SERVER_CODE
	auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
	RemoveGameplayEffectHandle(ASC);
#endif

	UE_AVVM_NOTIFY_IF_LOCALLYCONTROLLED(this,
	                                    StopPromptInteractionChannel,
	                                    PC,
	                                    Instigator,
	                                    FAVVMNotificationPayload::Empty);
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
