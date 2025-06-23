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

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
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
		UInteraction* MutableInteraction = Iterator->Get();
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
	TArray<UInteraction*> OldRecords = Records;

	auto* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewTarget, NewInstigator);

	AddReplicatedSubObject(Transaction);
	Records.Add(Transaction);

	OnRep_RecordModified(OldRecords);
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
		TArray<UInteraction*> OldRecords = Records;

		UInteraction* Transaction = SearchResult->Get();
		RemoveReplicatedSubObject(Transaction);
		Records.Remove(Transaction);

		OnRep_RecordModified(OldRecords);
	}
}

const AActor* UGameStateInteractionComponent::GetGameplayEffectCauser(const AActor* Instigator) const
{
	const FActiveGameplayEffectHandle* SearchResult = ActorToGEActiveHandle.Find(Instigator);
	if (SearchResult == nullptr)
	{
		return nullptr;
	}

	auto* ASC = SearchResult->GetOwningAbilitySystemComponent();
	if (IsValid(ASC))
	{
		const FGameplayEffectContextHandle GEContextHandle = ASC->GetEffectContextFromActiveGEHandle(*SearchResult);
		return GEContextHandle.GetEffectCauser();
	}

	return nullptr;
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
	const AActor* Target = TopRecord->GetTarget();

	if (IsValid(Instigator))
	{
		// @gdemers ApplyGFE on Server creates a racing condition between the server/client and trigger the ability BEFORE we updated the TMap caching GFEHandle on the Client (which we care about due
		// to the access to EffectCauser Actor in UPlayerInteractionAbility).
		// UPlayerInteractionAbility is currently set to Activation based on Tag added/removed for testing purposes but we actually care more about user input to trigger the ability so the racing condition
		// isnt a real issue here. However, this approach bothers me a little...
		// Note : Client Side GFEActivationHandle will be default initialized without data due to not being Actor ROLE_Authority. (This should be fine!)
		auto* ASC = Instigator->GetComponentByClass<UAbilitySystemComponent>();
		const FGameplayEffectSpecHandle GESpecHandle = UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(GameplayEffect, const_cast<AActor*>(Instigator), const_cast<AActor*>(Target));
		AddGameplayEffectHandle(ASC, GESpecHandle);
	}

	UE_AVVM_NOTIFY_LOCALPLAYER_ONLY(this,
	                                StartPromptInteractionChannel,
	                                Target,
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
	const AActor* Target = FoundMatch->GetTarget();

	if (IsValid(Instigator))
	{
		auto* ASC = Instigator->GetComponentByClass<UAbilitySystemComponent>();
		RemoveGameplayEffectHandle(ASC);
	}

	UE_AVVM_NOTIFY_LOCALPLAYER_ONLY(this,
	                                StopPromptInteractionChannel,
	                                Target,
	                                FAVVMNotificationPayload::Empty);
}

void UGameStateInteractionComponent::AddGameplayEffectHandle(UAbilitySystemComponent* ASC,
                                                             const FGameplayEffectSpecHandle& GEHandle)
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

void UGameStateInteractionComponent::RemoveGameplayEffectHandle(UAbilitySystemComponent* ASC)
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
