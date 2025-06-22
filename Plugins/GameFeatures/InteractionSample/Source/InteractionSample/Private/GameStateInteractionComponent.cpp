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
#include "GameStateInteractionComponent.h"

#include "AbilitySystemComponent.h"
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMNotificationSubsystem.h"
#include "Interaction.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UGameStateInteractionComponent::UGameStateInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UGameStateInteractionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameStateInteractionComponent, Records);
}

void UGameStateInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UGameStateInteractionComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	OwningOuter = Outer;
}

void UGameStateInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_SERVER_CODE
	for (auto Iterator = Records.CreateIterator(); Iterator; ++Iterator)
	{
		auto* MutableInteraction = const_cast<UInteraction*>(Iterator->Get());
		RemoveReplicatedSubObject(MutableInteraction);
		Iterator.RemoveCurrentSwap();
	}

	for (auto Iterator = Records.CreateIterator(); Iterator; ++Iterator)
	{
		auto* MutableInteraction = const_cast<UInteraction*>(Iterator->Get());
		RemoveReplicatedSubObject(MutableInteraction);
		Iterator.RemoveCurrentSwap();
	}
#endif

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UGameStateInteractionComponent from Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	OwningOuter.Reset();
}

UGameStateInteractionComponent* UGameStateInteractionComponent::GetActorComponent(const UObject* WorldContextObject)
{
	AGameStateBase* GameState = UGameplayStatics::GetGameState(WorldContextObject);
	if (IsValid(GameState))
	{
		return GameState->GetComponentByClass<UGameStateInteractionComponent>();
	}
	else
	{
		return nullptr;
	}
}

TArray<UInteraction*> UGameStateInteractionComponent::GetPartialMatchingInteractions(const AActor* NewTarget) const
{
	return Records.FilterByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesPartialMatch(NewTarget);
	});
}

TArray<UInteraction*> UGameStateInteractionComponent::GetExactMatchingInteractions(const AActor* NewTarget,
                                                                                   const AActor* NewInstigator) const
{
	return Records.FilterByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesExactMatch(NewTarget, NewInstigator);
	});
}

void UGameStateInteractionComponent::Server_AddRecord(const AActor* NewTarget,
                                                      const AActor* NewInstigator)
{
	auto* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewTarget, NewInstigator);

	AddReplicatedSubObject(Transaction);
	Records.Add(Transaction);

	OnRep_RecordModified(Records);
}

void UGameStateInteractionComponent::Server_RemoveRecord(const AActor* NewTarget,
                                                         const AActor* NewInstigator)
{
	const TObjectPtr<UInteraction>* SearchResult = Records.FindByPredicate([&](const UInteraction* Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesExactMatch(NewTarget, NewInstigator);
	});

	if (SearchResult != nullptr)
	{
		UInteraction* Transaction = SearchResult->Get();
		RemoveReplicatedSubObject(Transaction);
		Records.Remove(Transaction);

		OnRep_RecordModified(Records);
	}
}

void UGameStateInteractionComponent::OnRep_RecordModified(const TArray<UInteraction*>& OldRecords)
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

void UGameStateInteractionComponent::HandleNewRecord(const TArray<UInteraction*>& NewRecords)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Record Collection modified on Actor \"%s\". Adding!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
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
	if (!IsValid(Instigator) || !UAVVMGameplayUtils::IsLocallyControlled(Instigator))
	{
		return;
	}

	auto* AbilityComponent = Instigator->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AddLooseGameplayTags(GrantAbilityTags);
	}

	UE_AVVM_NOTIFY(this,
	               StartPromptInteractionChannel,
	               TopRecord->GetTarget(),
	               FAVVMNotificationPayload::Empty);
}

void UGameStateInteractionComponent::HandleOldRecord(const TArray<UInteraction*>& OldRecords)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Record Collection modified on Actor \"%s\". Removing!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
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
			return IsValid(Param) && Param->DoesExactMatch(OldRecord->GetTarget(), OldRecord->GetInstigator());
		});

		if (SearchResult == nullptr)
		{
			// @gdemers rip. no longer with us!
			FoundMatch = OldRecord;
			break;
		}
	}

	if (!ensureAlwaysMsgf(IsValid(FoundMatch),
	                      TEXT("UGameStateInteractionComponent::HandleOldRecord didnt find a match for removal!")))
	{
		return;
	}

	const AActor* Instigator = FoundMatch->GetInstigator();
	if (!IsValid(Instigator) || !UAVVMGameplayUtils::IsLocallyControlled(Instigator))
	{
		return;
	}

	auto* AbilityComponent = Instigator->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveLooseGameplayTags(GrantAbilityTags);
	}

	UE_AVVM_NOTIFY(this,
	               StopPromptInteractionChannel,
	               FoundMatch->GetTarget(),
	               FAVVMNotificationPayload::Empty);
}
