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

bool UActorInteractionImpl::HandleBeginOverlap(const TArray<UInteraction*>& NewRecords,
                                               const AActor* NewInstigator,
                                               const AActor* NewTarget,
                                               const bool bShouldPreventContingency)
{
	if (!IsValid(NewTarget))
	{
		return false;
	}

	const bool bHasActorAuthority = UAVVMGameplayUtils::CheckActorAuthority(NewTarget);
	if (!bHasActorAuthority)
	{
		return false;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return false;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Begin Overlap detected between Instigator \"%s\" and Target \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *NewInstigator->GetName(),
	       *NewTarget->GetName());

	return AttemptBeginOverlap(NewRecords,
	                           NewInstigator /*World Actor*/,
	                           bShouldPreventContingency);
}

bool UActorInteractionImpl::HandleEndOverlap(const TArray<UInteraction*>& NewRecords,
                                             const AActor* NewInstigator,
                                             const AActor* NewTarget)
{
	if (!IsValid(NewTarget))
	{
		return false;
	}

	const bool bHasActorAuthority = UAVVMGameplayUtils::CheckActorAuthority(NewTarget);
	if (!bHasActorAuthority)
	{
		return false;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return false;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". End Overlap detected between Instigator \"%s\" and Target \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *NewInstigator->GetName(),
	       *NewTarget->GetName());

	return AttemptEndOverlap(NewRecords,
	                         NewInstigator /*World Actor*/,
	                         NewTarget /*AController*/);
}

void UActorInteractionImpl::HandleRecordModified(const TArray<UInteraction*>& OldRecords,
                                                 const TArray<UInteraction*>& NewRecords)
{
	const int32 OldNum = OldRecords.Num();
	const int32 NewNum = NewRecords.Num();

	if (OldNum < NewNum)
	{
		HandleNewRecord(NewRecords);
	}
	else if (OldNum > NewNum)
	{
		HandleOldRecord(NewRecords, OldRecords);
	}
}

TArray<UInteraction*> UActorInteractionImpl::GetExactMatchingInteractions(const TArray<UInteraction*>& Records,
                                                                          const AActor* NewInstigator,
                                                                          const AActor* NewTarget) const
{
	return Records.FilterByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesExactMatch(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	});
}

TArray<UInteraction*> UActorInteractionImpl::GetPartialMatchingInteractions(const TArray<UInteraction*>& Records,
                                                                            const AActor* NewInstigator) const
{
	return Records.FilterByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesPartialMatch(NewInstigator);
	});
}

bool UActorInteractionImpl::AttemptBeginOverlap(const TArray<UInteraction*>& NewRecords,
                                                const AActor* NewInstigator,
                                                const bool bShouldPreventContingency)
{
	if (!bShouldPreventContingency)
	{
		return true;
	}

	bool bCanInteract = true;
	for (const UInteraction* Interaction : GetPartialMatchingInteractions(NewRecords, NewInstigator))
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

bool UActorInteractionImpl::AttemptEndOverlap(const TArray<UInteraction*>& NewRecords,
                                              const AActor* NewInstigator,
                                              const AActor* NewTarget)
{
	const auto MatchingInteractions = GetExactMatchingInteractions(NewRecords, NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	return !MatchingInteractions.IsEmpty();
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
	if (Outer->GetLocalRole() == ROLE_Authority)
	{
		auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
		const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(GameplayEffect,
		                                                                                                     PlayerState /*APlayerState*/,
		                                                                                                     const_cast<AActor*>(Instigator)/*World Actor*/);
		AddGameplayEffectHandle(ASC, GESpecHandle);
	}
#endif

	UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
	                                        StartPromptInteractionChannel,
	                                        PC,
	                                        Instigator,
	                                        FAVVMNotificationPayload::Empty);
}

void UActorInteractionImpl::HandleOldRecord(const TArray<UInteraction*>& NewRecords,
                                            const TArray<UInteraction*>& OldRecords)
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
	for (const UInteraction* OldRecord : OldRecords)
	{
		if (!IsValid(OldRecord))
		{
			continue;
		}

		const UInteraction* const* SearchResult = NewRecords.FindByPredicate([&](const UInteraction* Param)
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
	if (Outer->GetLocalRole() == ROLE_Authority)
	{
		auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState);
		RemoveGameplayEffectHandle(ASC);
	}
#endif

	UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
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
