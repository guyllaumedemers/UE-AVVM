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

#include "GameFramework/Info.h"

#include "InteractionManagerComponent.generated.h"

class UInteraction;

/**
*	Class Description :
 *
 *	UInteractionManagerComponent handle replication over network of interactions locally scheduled by clients. GFP _AddComponent is
 *	expected to Target the AGameState actor for creation.
 */
UCLASS(ClassGroup=("Interaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INTERACTIONSAMPLE_API UInteractionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionManagerComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure)
	static UInteractionManagerComponent* GetManager(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void AttemptRecordBeginOverlap(const AActor* NewTarget,
	                               const AActor* NewInstigator,
	                               const bool bPreventContingency);

	UFUNCTION(BlueprintCallable)
	void AttemptRecordEndOverlap(const AActor* NewTarget,
	                             const AActor* NewInstigator);

protected:
	UFUNCTION(Server, Unreliable)
	void ServerRPC_RecordBeginInteraction(const AActor* NewTarget,
	                                      const AActor* NewInstigator);

	UFUNCTION(Server, Unreliable)
	void ServerRPC_RecordEndInteraction(const AActor* NewTarget,
	                                    const AActor* NewInstigator);

	UFUNCTION()
	void OnRep_NewBeginInteractionRecorded();

	UFUNCTION()
	void OnRep_NewEndInteractionRecorded();

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewBeginInteractionRecorded")
	TArray<TObjectPtr<const UInteraction>> BeginInteractions;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_NewEndInteractionRecorded")
	TArray<TObjectPtr<const UInteraction>> EndInteractions;
};
