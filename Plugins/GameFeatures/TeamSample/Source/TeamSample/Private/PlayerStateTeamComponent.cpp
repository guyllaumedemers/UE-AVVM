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
#include "TeamObject.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UPlayerStateTeamComponent::UPlayerStateTeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UPlayerStateTeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStateTeamComponent, OwningTeam);
}

void UPlayerStateTeamComponent::BeginPlay()
{
	Super::BeginPlay();

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
	
	RemoveReplicatedSubObject(const_cast<UTeamObject*>(OwningTeam.Get()));

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

void UPlayerStateTeamComponent::Static_TrySwitchTeam(const APlayerState* NewPlayerState, const FGameplayTag& NewTeamTag)
{
	auto* TeamComponent = UPlayerStateTeamComponent::GetActorComponent(NewPlayerState);
	if (IsValid(TeamComponent))
	{
		TeamComponent->TrySwitchTeam(NewTeamTag);
	}
}

void UPlayerStateTeamComponent::Static_TryForfaiting(const APlayerState* NewPlayerState)
{
	auto* TeamComponent = UPlayerStateTeamComponent::GetActorComponent(NewPlayerState);
	if (IsValid(TeamComponent))
	{
		TeamComponent->TryForfaiting();
	}
}

UPlayerStateTeamComponent* UPlayerStateTeamComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UPlayerStateTeamComponent>() : nullptr;
}

void UPlayerStateTeamComponent::SetTeam(UTeamObject* NewTeam)
{
#if WITH_SERVER_CODE
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (Outer->HasAuthority())
	{
		// TODO @gdemers add registration to delegate on the UTeamObject to respond to events such as
		// votes, request to swap team, etc...
		RemoveReplicatedSubObject(const_cast<UTeamObject*>(OwningTeam.Get()));
		AddReplicatedSubObject(NewTeam);
		OwningTeam = NewTeam;
	}
#endif
}

void UPlayerStateTeamComponent::TrySwitchTeam_Implementation(const FGameplayTag& NewTeamTag)
{
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const UTeamObject* Team = OwningTeam.Get();
	if (!IsValid(Team))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Actor \"%s\" TrySwitchTeam from Team \"%s\" to Team Tag \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName(),
	       *Team->GetName(),
	       *NewTeamTag.ToString());
}

void UPlayerStateTeamComponent::TryForfaiting_Implementation()
{
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const UTeamObject* Team = OwningTeam.Get();
	if (!IsValid(Team))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Actor \"%s\" TryForfaiting from Team \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName(),
	       *Team->GetName());
}
