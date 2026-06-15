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
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"

#include "ActorInteractionImpl.generated.h"

struct FGameplayEffectSpecHandle;
struct FInteractionExecutionContext;
struct FInteractionExecutionRequirements;
class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class UInteraction;

/**
 *	Class description:
 *	
*	FInteractionImplSparseData is a Shared representation of a class object immutable data. It reduces memory footprint
 *	by removing the need to allocate that data on instanced class object, and instead reference the shared memory.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionImplSparseData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	FGameplayTag StartPromptInteractionChannel = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	FGameplayTag StopPromptInteractionChannel = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	TInstancedStruct<FInteractionExecutionRequirements> Requirements = TInstancedStruct<FInteractionExecutionRequirements>();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef))
	TInstancedStruct<FInteractionExecutionContext> ExecutionCtx = TInstancedStruct<FInteractionExecutionContext>();
};

/**
 *	Class Description :
 *
 *	UActorInteractionImpl is a functional class from which we run validation during a player interaction with a world actor.
 */
UCLASS(BlueprintType, Blueprintable, SparseClassDataTypes="InteractionImplSparseData")
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

	bool StartExecute(const AActor* NewInstigator,
	                  const AActor* NewTarget,
	                  const TArray<UInteraction*>& NewRecords,
	                  const bool bShouldPreventContingency);

	bool StopExecute(const AActor* NewInstigator,
	                 const AActor* NewTarget,
	                 const TArray<UInteraction*>& NewRecords,
	                 const bool bShouldPreventContingency);
	
	void PumpHeartbeat(const AActor* NewTarget, const float NewDelta) const;
	void Execute(const AActor* NewTarget) const;
	void Kill(const AActor* NewTarget) const;

	bool DoesMeetExecutionRequirements(const TInstancedStruct<FInteractionExecutionRequirements>& Compare) const;
	const TInstancedStruct<FInteractionExecutionRequirements>& GetExecutionRequirements() const;

#if WITH_EDITOR
	// ~ This function transfers existing data into FMySparseClassData.
	virtual void MoveDataToSparseClassDataStruct() const override;
#endif // WITH_EDITOR

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
	void HandlePendingKillRecords(const TArray<UInteraction*>& PendingKillRecords);

	void AddGameplayEffectHandle(UAbilitySystemComponent* ASC, const FGameplayEffectSpecHandle& GEHandle);
	void RemoveGameplayEffectHandle(UAbilitySystemComponent* ASC);

	bool Server_LockInteraction(const TArray<UInteraction*>& NewRecords,
	                            const AActor* NewInstigator,
	                            const AActor* NewTarget);

	bool Server_UnlockInteraction(const TArray<UInteraction*>& NewRecords,
	                              const AActor* NewInstigator,
	                              const AActor* NewTarget);

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const AActor>, FActiveGameplayEffectHandle> ActorToGEActiveHandle;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

private:
#if WITH_EDITORONLY_DATA
	//~ These properties are moving out to the FMySparseClassData struct:
	UPROPERTY()
	TSubclassOf<UGameplayEffect> GameplayEffect_DEPRECATED = nullptr;

	UPROPERTY()
	FGameplayTag StartPromptInteractionChannel_DEPRECATED = FGameplayTag::EmptyTag;

	UPROPERTY()
	FGameplayTag StopPromptInteractionChannel_DEPRECATED = FGameplayTag::EmptyTag;

	UPROPERTY()
	TInstancedStruct<FInteractionExecutionRequirements> Requirements_DEPRECATED = TInstancedStruct<FInteractionExecutionRequirements>();

	UPROPERTY()
	TInstancedStruct<FInteractionExecutionContext> ExecutionCtx_DEPRECATED = TInstancedStruct<FInteractionExecutionContext>();
#endif
};
