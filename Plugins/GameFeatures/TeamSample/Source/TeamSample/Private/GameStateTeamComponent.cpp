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

#include "GameStateTeamComponent.h"

#include "AVVMGameplayUtils.h"
#include "AVVMGameState.h"
#include "AVVMWorldSetting.h"
#include "PlayerStateTeamComponent.h"
#include "TeamRule.h"
#include "GameFramework/PlayerState.h"

UGameStateTeamComponent::UGameStateTeamComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bReplicateUsingRegisteredSubObjectList = false;
}

void UGameStateTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	UPlayerStateTeamComponent::OnTeamComponentInitialized.AddUObject(this, &UGameStateTeamComponent::OnPlayerStateTeamComponentInitialized);

	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const auto* WorldSetting = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	}

	auto* Outer = GetTypedOuter<AAVVMGameState>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding UGameStateTeamComponent to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	if (IsValid(Outer))
	{
		Outer->OnPlayerStateRemoved.AddUObject(this, &UGameStateTeamComponent::OnPlayerStateRemoved);
		Outer->OnPlayerStateAdded.AddUObject(this, &UGameStateTeamComponent::OnPlayerStateAdded);
	}

	OwningOuter = Outer;
}

void UGameStateTeamComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UPlayerStateTeamComponent::OnTeamComponentInitialized.RemoveAll(this);

	auto* Outer = Cast<AAVVMGameState>(OwningOuter.Get());
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing UGameStateTeamComponent from Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName());

	Outer->OnPlayerStateRemoved.RemoveAll(this);
	Outer->OnPlayerStateAdded.RemoveAll(this);
}

void UGameStateTeamComponent::OnPlayerStateAdded(APlayerState* PlayerState)
{
	auto* TeamComponent = UPlayerStateTeamComponent::GetActorComponent(PlayerState);
	if (IsValid(TeamComponent))
	{
		AssignTeam(TeamComponent);
	}
}

void UGameStateTeamComponent::OnPlayerStateRemoved(APlayerState* PlayerState)
{
	auto* TeamComponent = UPlayerStateTeamComponent::GetActorComponent(PlayerState);
	if (IsValid(TeamComponent))
	{
		UnAssignTeam(TeamComponent);
	}
}

void UGameStateTeamComponent::UnAssignTeam(UPlayerStateTeamComponent* NewPlayer)
{
	const UTeamRule* NewTeamRule = TeamRule.Get();
	if (IsValid(NewTeamRule))
	{
		NewTeamRule->HandleTeamRemoval(NewPlayer, Teams);
	}
}

void UGameStateTeamComponent::AssignTeam(UPlayerStateTeamComponent* NewPlayer)
{
	const UTeamRule* NewTeamRule = TeamRule.Get();
	if (IsValid(NewTeamRule))
	{
		NewTeamRule->HandleTeamAssignment(NewPlayer, Teams);
	}
}

void UGameStateTeamComponent::OnPlayerStateTeamComponentInitialized(UPlayerStateTeamComponent* NewPlayer)
{
	// @gdemers GFP will push dynamically the PlayerStateTeamComponent, as such, we may have to resolve to this
	// globally accessible delegate to execute 'this' call.
	AssignTeam(NewPlayer);
}
