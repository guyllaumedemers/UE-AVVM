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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"

#include "ActorInteractionComponent.generated.h"

class UAbilitySystemComponent;
class UActorInteractionImpl;
class UGameplayAbility;

/**
 *	Class description:
 */
UCLASS(ClassGroup=("Interaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INTERACTIONSAMPLE_API UActorInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorInteractionComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool Execute(const AActor* NewTarget, UGameplayAbility* OwningAbility) const;

protected:
	UFUNCTION()
	void OnPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	                                      AActor* OtherActor,
	                                      UPrimitiveComponent* OtherComp,
	                                      int32 OtherBodyIndex,
	                                      bool bFromSweep,
	                                      const FHitResult& SweepResult);

	UFUNCTION()
	void OnPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                                    AActor* OtherActor,
	                                    UPrimitiveComponent* OtherComp,
	                                    int32 OtherBodyIndex);

	void Server_AddRecord(const AActor* NewInstigator,
	                      const AActor* NewTarget);

	void Server_RemoveRecord(const AActor* NewInstigator,
	                         const AActor* NewTarget);

	UFUNCTION()
	void OnRep_RecordModified(TArray<UInteraction*> OldRecords);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldPreventContingency = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UActorInteractionImpl> InteractionImplClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UActorInteractionImpl> InteractionImpl = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_RecordModified")
	TArray<TObjectPtr<UInteraction>> Records;

	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
