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
#include "PlayerInteractionComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Interaction.h"
#include "GameStateInteractionComponent.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UPlayerInteractionComponent_InstanceCounter, TEXT("Player Interaction Component Instance Counter"));

UPlayerInteractionComponent::UPlayerInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	GameStateInteractionComponent = UGameStateInteractionComponent::GetActorComponent(this);

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UPlayerInteractionComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UPlayerInteractionComponent to PlayerState \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())

	OwningOuter = Outer;
}

void UPlayerInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GameStateInteractionComponent.Reset();

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UPlayerInteractionComponent to PlayerState \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

UPlayerInteractionComponent* UPlayerInteractionComponent::GetActorComponent(const AActor* Actor)
{
	if (IsValid(Actor))
	{
		const auto* PC = Actor->GetOwner<APlayerController>();
		return IsValid(PC) ? PC->GetComponentByClass<UPlayerInteractionComponent>() : nullptr;
	}
	else
	{
		return nullptr;
	}
}

void UPlayerInteractionComponent::AttemptRecordBeginOverlap(const AActor* NewInstigator,
                                                            const AActor* NewTarget,
                                                            const bool bPreventContingency)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(AuthoritativeInteractionComponent), TEXT("Invalid GameStateInteractionComponent!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Owning Connection \"%s\". Begin Overlap detected between Instigator \"%s\" and Target \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *UAVVMGameplayUtils::PrintConnectionInfo(Outer->GetNetConnection()),
	       *NewInstigator->GetName(),
	       *NewTarget->GetName());

	auto MatchingInteractions = TArray<UInteraction*>();
	bool bCanInteract = true;

	if (bPreventContingency)
	{
		MatchingInteractions = AuthoritativeInteractionComponent->GetPartialMatchingInteractions(NewInstigator);

		for (const UInteraction* Interaction : MatchingInteractions)
		{
			if (IsValid(Interaction) && !Interaction->CanInteract())
			{
				bCanInteract = false;
				break;
			}
		}
	}

	if (!bPreventContingency || MatchingInteractions.IsEmpty() || (bPreventContingency && bCanInteract))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Attempt Executing ServerRPC_RecordBeginInteraction..."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData());

		ServerRPC_RecordBeginInteraction(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	}
}

void UPlayerInteractionComponent::AttemptRecordEndOverlap(const AActor* NewInstigator,
                                                          const AActor* NewTarget)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(AuthoritativeInteractionComponent), TEXT("Invalid GameStateInteractionComponent!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Owning Connection \"%s\". End Overlap detected between Instigator \"%s\" and Target \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *UAVVMGameplayUtils::PrintConnectionInfo(Outer->GetNetConnection()),
	       *NewInstigator->GetName(),
	       *NewTarget->GetName());

	const auto MatchingInteractions = AuthoritativeInteractionComponent->GetExactMatchingInteractions(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	if (!MatchingInteractions.IsEmpty())
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Executing ServerRPC_RecordEndInteraction..."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData());

		ServerRPC_RecordEndInteraction(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
	}
}

void UPlayerInteractionComponent::ServerRPC_RecordBeginInteraction_Implementation(const AActor* NewInstigator,
                                                                                  const AActor* NewTarget)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(AuthoritativeInteractionComponent), TEXT("Invalid GameStateInteractionComponent!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". ServerRPC_RecordBeginInteraction executed on Actor \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	AuthoritativeInteractionComponent->Server_AddRecord(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
}

void UPlayerInteractionComponent::ServerRPC_RecordEndInteraction_Implementation(const AActor* NewInstigator,
                                                                                const AActor* NewTarget)
{
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	if (!ensureAlwaysMsgf(IsValid(AuthoritativeInteractionComponent), TEXT("Invalid GameStateInteractionComponent!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". ServerRPC_RecordEndInteraction executed on Actor \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	AuthoritativeInteractionComponent->Server_RemoveRecord(NewInstigator /*World Actor*/, NewTarget /*APlayerCharacter*/);
}
