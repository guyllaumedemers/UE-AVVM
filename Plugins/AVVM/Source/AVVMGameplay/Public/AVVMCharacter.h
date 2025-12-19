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
#include "AVVMSocketTargetingHelper.h"
#include "DataRegistryId.h"
#include "GameplayTagContainer.h"
#include "ModularCharacter.h"
#include "Data/AVVMActorPayload.h"
#include "Resources/AVVMResourceProvider.h"

#include "AVVMCharacter.generated.h"

class UAbilitySystemComponent;
class UAVVMAbilitySystemComponent;

/**
 *	Class description:
 *
 *	FAVVMCharacterChannelAggregator is a context struct that aggregate channels relevant to ACharacter
 *	and publish via the AVVMNotification subsystem to registered View Models.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMCharacterChannelAggregator
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PostPlayerControllerReplicationTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PostPlayerStateReplicationTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PostCharacterBeginTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PostCharacterEndTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	AAVVMCharacter is a derived impl of the ACharacter class that manage resource loading request and forward ASC request to its owning
 *	APlayerState.
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMCharacter : public AModularCharacter,
                                        public IAbilitySystemInterface,
                                        public IAVVMCanExposeActorPayload,
                                        public IAVVMDoesSupportSocketDeferral,
                                        public IAVVMResourceProvider
{
	GENERATED_BODY()

public:
	AAVVMCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	UAVVMAbilitySystemComponent* BP_GetAbilitySystemComponent() const;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// IAVVMCanExposeActorPayload
	virtual TInstancedStruct<FAVVMActorContext> GetExposedActorContext_Implementation() const override;
	
	// IAVVMDoesSupportSocketDeferral
	virtual void NotifyAvailableSocketParent(AActor* SocketTarget) override;

	// IAVVMResourceProvider
	virtual TArray<FDataRegistryId> GetResourceDefinitionResourceIds_Implementation() const override;
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const override;

protected:
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FAVVMCharacterChannelAggregator RegisteredChannels = FAVVMCharacterChannelAggregator();
	
	// @gdemers ActorDefinition allow Actor class overrides, so the ACharacter hierarchy could be
	// replaced at runtime if setup correctly to support character swapping.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorDefinitionDataTableRow"))
	FDataRegistryId ActorDefinitionId = FDataRegistryId();

	// @gdemers Resource Component handle initialization of our Character.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMResourceManagerComponent> ResourceManagerComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningActor = nullptr;
};
