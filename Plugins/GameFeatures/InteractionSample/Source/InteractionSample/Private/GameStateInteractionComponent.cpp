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

	DOREPLIFETIME(UGameStateInteractionComponent, BeginInteractions);
	DOREPLIFETIME(UGameStateInteractionComponent, EndInteractions);
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

TArray<TObjectPtr<const UInteraction>> UGameStateInteractionComponent::GetPartialMatchingInteractions(const AActor* NewTarget) const
{
	return BeginInteractions.FilterByPredicate([&](const TObjectPtr<const UInteraction>& Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesPartialMatch(NewTarget);
	});
}

TArray<TObjectPtr<const UInteraction>> UGameStateInteractionComponent::GetExactMatchingInteractions(const AActor* NewTarget,
                                                                                                    const AActor* NewInstigator) const
{
	return BeginInteractions.FilterByPredicate([&](const TObjectPtr<const UInteraction>& Interaction)
	{
		return IsValid(Interaction) && Interaction->DoesExactMatch(NewTarget, NewInstigator);
	});
}

void UGameStateInteractionComponent::Server_AddBeginOverlaped(UInteraction* NewInteraction)
{
	AddReplicatedSubObject(NewInteraction);
	BeginInteractions.Add(NewInteraction);

	OnRep_NewBeginInteractionRecorded();
}

void UGameStateInteractionComponent::Server_AddEndOverlaped(UInteraction* NewInteraction)
{
	AddReplicatedSubObject(NewInteraction);
	EndInteractions.Add(NewInteraction);

	OnRep_NewEndInteractionRecorded();

	const TObjectPtr<const UInteraction>* SearchResult = BeginInteractions.FindByPredicate([&](const TObjectPtr<const UInteraction>& Interaction)
	{
		return IsValid(Interaction) && Interaction->IsEqual(NewInteraction);
	});

	if (SearchResult != nullptr)
	{
		auto* Target = const_cast<UInteraction*>(SearchResult->Get());
		RemoveReplicatedSubObject(Target);
		BeginInteractions.Remove(Target);
	}
}

void UGameStateInteractionComponent::OnRep_NewBeginInteractionRecorded()
{
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Begin Interaction Collection modified on Actor \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	if (BeginInteractions.IsEmpty())
	{
		return;
	}

	const AActor* Instigator = BeginInteractions.Top()->GetInstigator();
	if (!IsValid(Instigator) || !UAVVMGameplayUtils::IsLocallyControlled(Instigator))
	{
		return;
	}

	auto* AbilityComponent = Instigator->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AddLooseGameplayTags(GrantAbilityTags);
	}
}

void UGameStateInteractionComponent::OnRep_NewEndInteractionRecorded()
{
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". End Interaction Collection modified on Actor \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	if (EndInteractions.IsEmpty())
	{
		return;
	}

	const AActor* Instigator = EndInteractions.Top()->GetInstigator();
	if (!IsValid(Instigator) || !UAVVMGameplayUtils::IsLocallyControlled(Instigator))
	{
		return;
	}

	auto* AbilityComponent = Instigator->GetComponentByClass<UAbilitySystemComponent>();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveLooseGameplayTags(GrantAbilityTags);
	}
}
