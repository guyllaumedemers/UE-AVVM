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
#include "AVVMModularActor.h"
#include "AVVMSocketTargetingHelper.h"
#include "GameplayTagContainer.h"
#include "Ability/AVVMAttributeSet.h"
#include "GameFramework/Actor.h"

#include "TriggeringAttachmentActor.generated.h"

/**
 *	Class description:
 *	
 *	FTriggeringAttachmentSocketTargetingHelper is a context struct that defines how an attachment should socket itself based
 *	on the known root actor (i.e ACharacter).
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FTriggeringAttachmentSocketTargetingHelper : public FAVVMSocketTargetingHelper
{
	GENERATED_BODY()

	virtual AActor* GetDesiredTypedInner(AActor* Src) const override;
};

/**
 *	Class description:
 *
 *	UTriggeringAttachmentComponent is an attachment system that extends the Outer Actor and can be invalidated
 *	during Unequip phase if required (since they are children of the actor with Authoritative state).
 */
UCLASS(BlueprintType, Blueprintable)
class WEAPONSAMPLE_API ATriggeringAttachmentActor : public AAVVMModularActor,
                                                    public IAbilitySystemInterface,
                                                    public IAVVMDoesOwnAttributeSet,
                                                    public IAVVMDoesSupportInnerSocketTargeting
{
	GENERATED_BODY()

public:
	ATriggeringAttachmentActor(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void Attach(AActor* NewParent);

	UFUNCTION(BlueprintCallable)
	void Detach();

	// @gdemers IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// @gdemers IAVVMDoesOwnAttributeSet
	virtual void SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet) override;
	
	// @gdemers IDoesSupportSocketTargeting
	virtual TInstancedStruct<FAVVMSocketTargetingHelper> GetSocketHelper_Implementation() const override;
	virtual void DeferredSocketParenting_Implementation(AActor* Dest) override;

protected:
	UFUNCTION()
	void OnSocketParentingDeferred(AActor* Parent);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag SlotTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FName SocketName = NAME_None;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<const UAttributeSet> OwnedAttributeSet = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
	
	FDelegateHandle DeferredSocketParentingDelegateHandle;

	friend class UAttachmentManagerComponent;
};
