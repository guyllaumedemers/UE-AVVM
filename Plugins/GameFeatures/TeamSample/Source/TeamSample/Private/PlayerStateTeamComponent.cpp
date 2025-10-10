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
#include "PlayerStateTeamComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Net/UnrealNetwork.h"

UPlayerStateTeamComponent::UPlayerStateTeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UPlayerStateTeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStateTeamComponent, TeamTag);
}

void UPlayerStateTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		OnTeamComponentInitialized.Broadcast(this);
	}

	auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UPlayerStateTeamComponent to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	OwningOuter = Outer;
}

void UPlayerStateTeamComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UPlayerStateTeamComponent from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());
}

UPlayerStateTeamComponent* UPlayerStateTeamComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UPlayerStateTeamComponent>() : nullptr;
}

void UPlayerStateTeamComponent::TrySwitchTeam_Implementation(const FGameplayTag& NewTeamTag)
{
	// TODO @gdemers add impl
}

void UPlayerStateTeamComponent::TryForfaiting_Implementation()
{
}

void UPlayerStateTeamComponent::SetTeam(const FGameplayTag& NewTeamTag)
{
	TeamTag = NewTeamTag;
}

void UPlayerStateTeamComponent::OnRep_OnTeamChanged(const FGameplayTag& OldTeamTag)
{
}
