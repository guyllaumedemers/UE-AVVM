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
#include "UObject/Object.h"

#include "ActorInteractionImpl.generated.h"

struct FGameplayEffectSpecHandle;
class UAbilitySystemComponent;
class UGameplayEffect;
class UInteraction;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class INTERACTIONSAMPLE_API UActorInteractionImpl : public UObject
{
	GENERATED_BODY()

public:
	virtual void SafeBegin();
	virtual void SafeEnd();

	bool HandleBeginOverlap(const TArray<UInteraction*>& NewRecords,
	                        const AActor* NewInstigator /*World Actor*/,
	                        const AActor* NewTarget /*APlayerCharacter*/,
	                        const bool bShouldPreventContingency);

	bool HandleEndOverlap(const TArray<UInteraction*>& NewRecords,
	                      const AActor* NewInstigator /*World Actor*/,
	                      const AActor* NewTarget /*APlayerCharacter*/);

	void HandleRecordModified(const TArray<UInteraction*>& OldRecords,
	                          const TArray<UInteraction*>& NewRecords);

protected:
	TArray<UInteraction*> GetExactMatchingInteractions(const TArray<UInteraction*>& Records,
	                                                   const AActor* NewInstigator,
	                                                   const AActor* NewTarget) const;

	TArray<UInteraction*> GetPartialMatchingInteractions(const TArray<UInteraction*>& Records,
	                                                     const AActor* NewInstigator) const;

	virtual bool AttemptBeginOverlap(const TArray<UInteraction*>& NewRecords,
	                                 const AActor* NewInstigator,
	                                 const bool bShouldPreventContingency);

	virtual bool AttemptEndOverlap(const TArray<UInteraction*>& NewRecords,
	                               const AActor* NewInstigator,
	                               const AActor* NewTarget);

	void HandleNewRecord(const TArray<UInteraction*>& NewRecords);
	void HandleOldRecord(const TArray<UInteraction*>& NewRecords,
	                     const TArray<UInteraction*>& OldRecords);

	void AddGameplayEffectHandle(UAbilitySystemComponent* ASC, const FGameplayEffectSpecHandle& GEHandle);
	void RemoveGameplayEffectHandle(UAbilitySystemComponent* ASC);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StartPromptInteractionChannel = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StopPromptInteractionChannel = FGameplayTag::EmptyTag;

	TMap<TWeakObjectPtr<const AActor>, FActiveGameplayEffectHandle> ActorToGEActiveHandle;
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
