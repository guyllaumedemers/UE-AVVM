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
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UPlayerInteractionComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UPlayerInteractionComponent to Actor \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName())

	OwningOuter = Outer;
}

void UPlayerInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GameStateInteractionComponent.Reset();

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UPlayerInteractionComponent to Actor \"%s\"."),
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

void UPlayerInteractionComponent::AttemptRecordBeginOverlap(const AActor* NewTarget,
                                                            const AActor* NewInstigator,
                                                            const bool bPreventContingency)
{
	const auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer) && IsValid(AuthoritativeInteractionComponent), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Owning Connection \"%s\". Begin Overlap detected between Actors \"%s\" and \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *UAVVMGameplayUtils::PrintConnectionInfo(Outer->GetNetConnection()),
	       *NewTarget->GetName(),
	       *NewInstigator->GetName());

	auto MatchingInteractions = TArray<TObjectPtr<const UInteraction>>();
	if (bPreventContingency)
	{
		MatchingInteractions = AuthoritativeInteractionComponent->GetMatchingInteractions(NewTarget);
	}

	if (!bPreventContingency || MatchingInteractions.IsEmpty())
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Attempt Executing ServerRPC_RecordBeginInteraction..."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData());

		ServerRPC_RecordBeginInteraction(NewTarget, NewInstigator);
	}
}

void UPlayerInteractionComponent::AttemptRecordEndOverlap(const AActor* NewTarget,
                                                          const AActor* NewInstigator)
{
	const auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer) && IsValid(AuthoritativeInteractionComponent), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Owning Connection \"%s\". End Overlap detected between Actors \"%s\" and \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *UAVVMGameplayUtils::PrintConnectionInfo(Outer->GetNetConnection()),
	       *NewTarget->GetName(),
	       *NewInstigator->GetName());

	const auto MatchingInteractions = AuthoritativeInteractionComponent->GetMatchingInteractions(NewTarget);
	if (!MatchingInteractions.IsEmpty())
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Executing ServerRPC_RecordEndInteraction..."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData());

		ServerRPC_RecordEndInteraction(NewTarget, NewInstigator);
	}
}

void UPlayerInteractionComponent::ServerRPC_RecordBeginInteraction_Implementation(const AActor* NewTarget,
                                                                                  const AActor* NewInstigator)
{
	auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer) && IsValid(AuthoritativeInteractionComponent), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". ServerRPC_RecordBeginInteraction executed on Actor \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	UInteraction* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewTarget, NewInstigator);
	AuthoritativeInteractionComponent->Server_AddBeginOverlaped(Transaction);
}

void UPlayerInteractionComponent::ServerRPC_RecordEndInteraction_Implementation(const AActor* NewTarget,
                                                                                const AActor* NewInstigator)
{
	auto* AuthoritativeInteractionComponent = GameStateInteractionComponent.Get();
	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer) && IsValid(AuthoritativeInteractionComponent), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". ServerRPC_RecordEndInteraction executed on Actor \"%s\"!"),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	UInteraction* Transaction = NewObject<UInteraction>(this);
	Transaction->operator()(NewTarget, NewInstigator);
	AuthoritativeInteractionComponent->Server_AddEndOverlaped(Transaction);
}
