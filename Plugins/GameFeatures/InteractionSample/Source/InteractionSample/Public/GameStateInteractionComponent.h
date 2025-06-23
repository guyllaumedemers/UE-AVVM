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

#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Info.h"

#include "GameStateInteractionComponent.generated.h"

struct FGameplayEffectSpecHandle;
class UAbilitySystemComponent;
class UGameplayEffect;
class UInteraction;

/**
 *	Class Description :
 *
*	UGameStateInteractionComponent store/update replicated user interactions based on client RPC requests.
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

	TArray<UInteraction*> GetPartialMatchingInteractions(const AActor* NewTarget) const;
	TArray<UInteraction*> GetExactMatchingInteractions(const AActor* NewTarget, const AActor* NewInstigator) const;

	UFUNCTION(BlueprintCallable)
	void Server_AddRecord(const AActor* NewTarget,
	                      const AActor* NewInstigator);

	UFUNCTION(BlueprintCallable)
	void Server_RemoveRecord(const AActor* NewTarget,
	                         const AActor* NewInstigator);

	UFUNCTION(BlueprintCallable)
	const AActor* GetGameplayEffectCauser(const AActor* Instigator) const;

protected:
	UFUNCTION()
	void OnRep_RecordModified(const TArray<UInteraction*>& OldRecords);

	void HandleNewRecord(const TArray<UInteraction*>& NewRecords);
	void HandleOldRecord(const TArray<UInteraction*>& OldRecords);

	void AddGameplayEffectHandle(UAbilitySystemComponent* ASC,
	                             const FGameplayEffectSpecHandle& GEHandle);

	void RemoveGameplayEffectHandle(UAbilitySystemComponent* ASC);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StartPromptInteractionChannel = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StopPromptInteractionChannel = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_RecordModified")
	TArray<TObjectPtr<UInteraction>> Records;

	TMap<TWeakObjectPtr<const AActor>, FActiveGameplayEffectHandle> ActorToGEActiveHandle;
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
