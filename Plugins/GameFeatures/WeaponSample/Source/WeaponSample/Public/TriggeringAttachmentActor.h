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

#include "DataRegistryId.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"

#include "TriggeringAttachmentActor.generated.h"

/**
 *	Class description:
 *
 *	UTriggeringAttachmentComponent is an attachment system that extends the Outer Actor and can be invalidated
 *	during Unequip phase if required (since they are children of the actor with Authoritative state).
 *
 *	TODO @gdemers Currently doesn't have a way to prevent calling _AddComponent on an Actor that's not a ATriggeringActor
 *	but will add proper validation later for this case.
 */
UCLASS()
class WEAPONSAMPLE_API ATriggeringAttachmentActor : public AActor
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void RegisterGameplayEffects(UAbilitySystemComponent* NewAbilitySystemComponent,
	                             const TArray<UObject*>& NewResources);

	UFUNCTION(BlueprintCallable)
	void UnRegisterGameplayEffects();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDataRegistryId AttachmentModifierDefinitionId = FDataRegistryId();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<AActor> OwningOuter = nullptr;

	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> GameplayEffectSpecHandles;

	friend class UAttachmentManagerComponent;
};
