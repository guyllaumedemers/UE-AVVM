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
#include "GameFramework/Info.h"

#include "GameStateInteractionComponent.generated.h"

class UInteraction;

/**
 *	Class Description :
 *
*	UGameStateInteractionComponent store/update replicated user interactions based on client RPC requests.
 *
 *	Note : Pushed via GFP_AddComponents
 */
UCLASS(ClassGroup=("Interaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INTERACTIONSAMPLE_API UGameStateInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameStateInteractionComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UGameStateInteractionComponent* GetActorComponent(const UObject* WorldContextObject);

	TArray<TObjectPtr<const UInteraction>> GetMatchingInteractions(const AActor* NewTarget) const;

	UFUNCTION(BlueprintCallable)
	void Server_AddBeginOverlaped(UInteraction* NewInteraction);

	UFUNCTION(BlueprintCallable)
	void Server_AddEndOverlaped(UInteraction* NewInteraction);

protected:
	UFUNCTION()
	void OnRep_NewBeginInteractionRecorded();

	UFUNCTION()
	void OnRep_NewEndInteractionRecorded();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer GrantAbilityTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewBeginInteractionRecorded")
	TArray<TObjectPtr<const UInteraction>> BeginInteractions;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewEndInteractionRecorded")
	TArray<TObjectPtr<const UInteraction>> EndInteractions;

	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
