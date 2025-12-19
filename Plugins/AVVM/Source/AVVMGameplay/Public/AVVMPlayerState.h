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

#include "AbilitySystemInterface.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMQuicktimeEventInterface.h"
#include "ModularPlayerState.h"
#include "Data/AVVMActorPayload.h"

#include "AVVMPlayerState.generated.h"

class IAVVMDoesImplNetSynchronization;
class UAVVMAbilitySystemComponent;

/**
 *	Class description:
 *
 *	FAVVMPlayerStateChannelAggregator is a context struct that aggregate channels relevant to APlayerState
 *	and publish via the AVVMNotification subsystem to registered View Models.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMPlayerStateChannelAggregator
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PostPlayerControllerClientInitializedTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerStatePayload is a context payload that forward player state being added or removed
 *	from the game.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMPlayerStatePayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMPlayerStatePayload() = default;
	explicit FAVVMPlayerStatePayload(const APlayerState* NewPlayerState,
	                                 const bool bNewAddOrRemove);

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const APlayerState> PlayerState = nullptr;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bWasAddedOrRemoved = false;
};

/**
 *	Class description:
 *
 *	AAVVMPlayerState is a GFP Receiver who registers with the GameFeatureFramework and react to GFP actions. It expects to receive system specific components depending on your project needs.
 *	
 *	IMPORTANT : Replicated systems SHOULD BE managed through a component added via GFP. Unreal's USubsystem derived classes CANNOT be replicated! Doing this approach makes for a more modular system
 *	due to component addition and removal no longer requiring hard references in the Actor derived class.
 *
 *	Note : for your UI systems. We expect to push an AVVMComponent with a pre-defined list of UAVVMPresenter class on it! Additionally, by implementing IAVVMQuicktimeEventPlayerStateInterface base
 *	api, you will be able to forward gameplay information without creating dependencies between modules. However, it comes with a price due to interface dispatch!
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMPlayerState : public AModularPlayerState,
                                          public IAbilitySystemInterface,
                                          public IAVVMCanExposeActorPayload,
                                          public IAVVMQuicktimeEventPlayerStateInterface
{
	GENERATED_BODY()
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostNetClientSynchronizationCompleteDelegate, const AAVVMPlayerState* PlayerState);
	FOnPostNetClientSynchronizationCompleteDelegate OnPostNetClientSynchronizationComplete;

public:
	AAVVMPlayerState(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void ClientInitialize(class AController* C) override;

	// @gdemers PlayerState is the preferred place to host the ASC as OnPawnPosses can be used to modify the internal state of the
	// ASC (good for state persistency between Pawn possession!).
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// IAVVMCanExposeActorPayload
	virtual TInstancedStruct<FAVVMActorContext> GetExposedActorContext_Implementation() const override;
	
	// @gdemers 
	FOnPostNetClientSynchronizationCompleteDelegate& GetOnPostNetClientSynchronizationComplete();

protected:
	UFUNCTION(Server, Reliable)
	void Server_OnClientPlayerControllerReceived();
	
	UFUNCTION(Client, Reliable)
	void Client_OnNetFinalized(const TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>>& NetFinalized);
	
	void HandleSimulatedPlayerBackfilling();
	
	UFUNCTION(Server, Reliable)
	void Server_OnPlayerBackfillingReceived(AAVVMPlayerState* SimulatedPlayerState);

	UFUNCTION(Client, Reliable)
	void Client_OnSimulatedClientNetFinalized(const TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>>& NetFinalized,
	                                          AAVVMPlayerState* SimulatedPlayerState);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FAVVMPlayerStateChannelAggregator RegisteredChannels = FAVVMPlayerStateChannelAggregator();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMAbilitySystemComponent> AbilitySystemComponent = nullptr;
};
