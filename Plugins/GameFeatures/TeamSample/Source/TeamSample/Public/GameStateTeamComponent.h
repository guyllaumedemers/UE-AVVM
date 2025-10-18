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

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"

#include "GameStateTeamComponent.generated.h"

class APlayerState;
struct FAVVMPartyProxy;
class UPlayerStateTeamComponent;
class UTeamObject;
class UTeamRule;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnBackendTeamRequestCompleteDelegate, const bool, bWasSuccess, const TArray<FAVVMPartyProxy>&, NewParties);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnReplicatedTeamChangedDelegate, const TArray<UTeamObject*>& NewTeams, const TArray<UTeamObject*>& OldTeams);

/**
 *	Class description:
 *
 *	UGameStateTeamComponent is a server-client component that store the team information retrieved from backend, and expose features defined by user provided Ruleset
 *	referenced in AWorldSetting.
 */
UCLASS(ClassGroup=("Team"), Blueprintable, meta=(BlueprintSpawnableComponent))
class TEAMSAMPLE_API UGameStateTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameStateTeamComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FOnReplicatedTeamChangedDelegate OnReplicatedTeamChanged;

protected:
	UFUNCTION()
	void OnPlayerStateAdded(APlayerState* NewPlayerState);

	UFUNCTION()
	void OnPlayerStateRemoved(APlayerState* NewPlayerState);

	UFUNCTION()
	void OnPlayerStateTeamComponentInitialized(APlayerState* NewPlayerState);

	void GetTeamRuleOnAuthority();
	void GetBackendTeams(const FOnBackendTeamRequestCompleteDelegate& Callback);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_RequestGameSesionParties(const FName SessionName, const FOnBackendTeamRequestCompleteDelegate& Callback);

	UFUNCTION()
	void OnTeamReceived(const bool bWasSuccess, const TArray<FAVVMPartyProxy>& NewParties);

	UFUNCTION()
	void OnRep_OnTeamChanged(const TArray<UTeamObject*>& OldTeams);

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UTeamRule> TeamRule = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_OnTeamChanged")
	TArray<TObjectPtr<UTeamObject>> Teams;

	UPROPERTY(Transient, meta=(ToolTip="Players waiting to be assigned to a team based on data from backend."))
	TArray<TWeakObjectPtr<APlayerState>> PendingPlayers;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<AActor> OwningOuter = nullptr;
};
