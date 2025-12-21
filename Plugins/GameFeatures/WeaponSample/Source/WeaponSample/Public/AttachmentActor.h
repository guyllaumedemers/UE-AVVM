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
#include "Ability/AVVMAttributeSet.h"
#include "GameFramework/Actor.h"

#include "AttachmentActor.generated.h"

/**
 *	Class description:
 *	
 *	FAttachmentSocketTargetingHelper is a context struct that defines how an attachment should socket itself based
 *	on the known root actor (i.e ACharacter).
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FAttachmentSocketTargetingHelper : public FAVVMSocketTargetingHelper
{
	GENERATED_BODY()

	virtual AActor* GetDesiredTypedInner(AActor* Src, AActor* Target) const override;
};

/**
 *	Class description:
 *
 *	AAttachmentActor is an attachment system that extends the Outer Actor and can be invalidated
 *	during Unequip phase if required (since they are children of the actor with Authoritative state).
 */
UCLASS(BlueprintType, Blueprintable)
class WEAPONSAMPLE_API AAttachmentActor : public AAVVMModularActor,
                                          public IAbilitySystemInterface,
                                          public IAVVMDoesOwnAttributeSet,
                                          public IAVVMDoesSupportInnerSocketTargeting
{
	GENERATED_BODY()

public:
	AAttachmentActor(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// @gdemers IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// @gdemers IAVVMDoesOwnAttributeSet
	virtual void SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet) override;

	// @gdemers IDoesSupportSocketTargeting
	virtual TInstancedStruct<FAVVMSocketTargetingHelper> GetSocketHelper_Implementation() const override;
	virtual void DeferredSocketParenting_Implementation(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs) override;
	virtual void Attach_Implementation(AActor* Target, const FName NewSocketName) override;
	virtual void Detach_Implementation() override;

protected:
	UFUNCTION()
	void OnSocketParentingDeferred(AActor* Parent,
	                               AActor* Target,
	                               const FAVVMSocketTargetingDeferralContextArgs ContextArgs);

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<const UAttributeSet> OwnedAttributeSet = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	FDelegateHandle DeferredSocketParentingDelegateHandle;

private:
	// @gdemers This property handles the attachment to a socket when the element is built-in the owning triggering actors.
	// This imply that the attachment arent part of the inventory system. They are baked into the representation of its owning actor, and attached at runtime (like a Gun blueprint).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	FName SocketName = NAME_None;

	friend class UAttachmentManagerComponent;
};
