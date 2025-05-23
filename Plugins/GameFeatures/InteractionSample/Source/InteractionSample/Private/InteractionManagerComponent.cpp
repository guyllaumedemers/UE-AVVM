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
#include "InteractionManagerComponent.h"

#include "Interaction.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UInteractionManagerComponent::UInteractionManagerComponent(const FObjectInitializer& ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInteractionManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionManagerComponent, BeginInteractions);
	DOREPLIFETIME(UInteractionManagerComponent, EndInteractions);
}

void UInteractionManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_SERVER_CODE
	for (auto Iterator = BeginInteractions.CreateIterator(); Iterator; ++Iterator)
	{
		auto* MutableInteraction = const_cast<UInteraction*>(Iterator->Get());
		RemoveReplicatedSubObject(MutableInteraction);
		Iterator.RemoveCurrentSwap();
	}

	for (auto Iterator = EndInteractions.CreateIterator(); Iterator; ++Iterator)
	{
		auto* MutableInteraction = const_cast<UInteraction*>(Iterator->Get());
		RemoveReplicatedSubObject(MutableInteraction);
		Iterator.RemoveCurrentSwap();
	}
#endif
}

UInteractionManagerComponent* UInteractionManagerComponent::GetManager(const UObject* WorldContextObject)
{
	AGameStateBase* GameStateBase = UGameplayStatics::GetGameState(WorldContextObject);
	if (IsValid(GameStateBase))
	{
		return GameStateBase->GetComponentByClass<UInteractionManagerComponent>();
	}
	else
	{
		return nullptr;
	}
}

void UInteractionManagerComponent::AttemptRecordBeginOverlap(const AActor* NewTarget,
                                                             const AActor* NewInstigator,
                                                             const bool bPreventContingency)
{
	UE_LOG(LogTemp, Log, TEXT("Client: Begin Overlap detected! Can we proceed ?"));

	auto MatchingInteractions = TArray<TObjectPtr<const UInteraction>>();
	if (bPreventContingency)
	{
		MatchingInteractions = BeginInteractions.FilterByPredicate([&](const TObjectPtr<const UInteraction>& Interaction)
		{
			return IsValid(Interaction) && Interaction->DoesPartialMatch(NewTarget);
		});
	}

	if (!bPreventContingency || MatchingInteractions.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("No match found! Or we dont care about Contingency! Attempt Executing ServerRPC_RecordBeginInteraction..."));
		ServerRPC_RecordBeginInteraction(NewTarget, NewInstigator);
	}
}

void UInteractionManagerComponent::AttemptRecordEndOverlap(const AActor* NewTarget,
                                                           const AActor* NewInstigator)
{
	UE_LOG(LogTemp, Log, TEXT("Client: End Overlap detected! Can we proceed ?"));
	const auto MatchingInteractions = BeginInteractions.FilterByPredicate([&](const TObjectPtr<const UInteraction>& Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesMatch(NewTarget, NewInstigator);
	});

	if (!MatchingInteractions.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Match found! Attempt Executing ServerRPC_RecordEndInteraction..."));
		ServerRPC_RecordEndInteraction(NewTarget, NewInstigator);
	}
}

void UInteractionManagerComponent::ServerRPC_RecordBeginInteraction_Implementation(const AActor* NewTarget,
                                                                                   const AActor* NewInstigator)
{
	UE_LOG(LogTemp, Log, TEXT("ServerRPC executed! New Begin Interaction Recorded!"));

	UInteraction* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewTarget, NewInstigator);
	AddReplicatedSubObject(Transaction);
	BeginInteractions.Add(Transaction);
}

void UInteractionManagerComponent::ServerRPC_RecordEndInteraction_Implementation(const AActor* NewTarget,
                                                                                 const AActor* NewInstigator)
{
	UE_LOG(LogTemp, Log, TEXT("ServerRPC executed! New End Interaction Recorded!"));

	UInteraction* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewTarget, NewInstigator);
	AddReplicatedSubObject(Transaction);
	EndInteractions.Add(Transaction);
}

void UInteractionManagerComponent::OnRep_NewBeginInteractionRecorded()
{
	UE_LOG(LogTemp, Log, TEXT("Begin Interaction Collection modified!"));
}

void UInteractionManagerComponent::OnRep_NewEndInteractionRecorded()
{
	UE_LOG(LogTemp, Log, TEXT("End Interaction Collection modified!"));
}
