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

#include "PlayerStateTeamComponent.generated.h"

/**
 *	Class description:
 *
 *	UPlayerStateTeamComponent is a server-client component that identify APlayerState parenting to a remote Team. User
 *	requests for team actions should be piped through this component.
 */
UCLASS(ClassGroup=("Team"), Blueprintable, meta=(BlueprintSpawnableComponent))
class TEAMSAMPLE_API UPlayerStateTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStateTeamComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UPlayerStateTeamComponent* GetActorComponent(const AActor* NewActor);

	UFUNCTION(BlueprintCallable)
	void SetTeam(const FGameplayTag& NewTeamTag);

	UFUNCTION(Server, Reliable)
	void TrySwitchTeam(const FGameplayTag& NewTeamTag);

	UFUNCTION(Server, Reliable)
	void TryForfaiting();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnTeamComponentInitializedDelegate, UPlayerStateTeamComponent*);
	static FOnTeamComponentInitializedDelegate OnTeamComponentInitialized;

protected:
	UFUNCTION()
	void OnRep_OnTeamChanged(const FGameplayTag& OldTeamTag);

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_OnTeamChanged")
	FGameplayTag TeamTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
