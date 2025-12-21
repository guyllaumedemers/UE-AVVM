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
#include "GameplayAbilitySpecHandle.h"
#include "InstancedStruct.h"
#include "Ability/AVVMAttributeSet.h"
#include "Backend/AVVMDataResolverHelper.h"
#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceProvider.h"

#include "TriggeringActor.generated.h"

struct FStreamableHandle;
class UAVVMAbilitySystemComponent;
class UTriggeringAbility;

/**
 *	Class description:
 *	
 *	FTriggeringActorDataResolverHelper is a context struct that resolve backend information about a triggering actor.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FTriggeringActorDataResolverHelper : public FAVVMDataResolverHelper
{
	GENERATED_BODY()
	
	virtual TArray<int32> GetElementDependencies(const UObject* WorldContextObject, const int32 ElementId) const override;
};

/**
 *	Class description:
 *	
 *	FTriggeringSocketTargetingHelper is a context struct that defines how an attachment should socket itself based
 *	on the known root actor (i.e ACharacter).
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FTriggeringSocketTargetingHelper : public FAVVMSocketTargetingHelper
{
	GENERATED_BODY()

	virtual AActor* GetDesiredTypedInner(AActor* Src, AActor* Target) const override;
};

/**
 *	Class description:
 *
 *	ATriggeringActor is a triggering system that executes behaviour such as triggering or targeting.
 */
UCLASS(BlueprintType, Blueprintable)
class WEAPONSAMPLE_API ATriggeringActor : public AAVVMModularActor,
                                          public IAbilitySystemInterface,
                                          public IAVVMDoesOwnAttributeSet,
                                          public IAVVMDoesSupportInnerSocketTargeting,
                                          public IAVVMResourceProvider
{
	GENERATED_BODY()

public:
	ATriggeringActor(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Trigger() const;
	virtual void Trigger_Implementation() const PURE_VIRTUAL(Trigger_Implementation, return;);
	
	// @gdemers IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// @gdemers IAVVMDoesOwnAttributeSet
	virtual void SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet) override;

	// @gdemers IDoesSupportSocketTargeting
	virtual TInstancedStruct<FAVVMSocketTargetingHelper> GetSocketHelper_Implementation() const override;
	virtual void DeferredSocketParenting_Implementation(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs) override;
	virtual void Attach_Implementation(AActor* Target, const FName NewSocketName) override;
	virtual void Detach_Implementation() override;

	// @gdemers IAVVMResourceProvider
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const override;
	virtual TArray<FDataRegistryId> GetResourceDefinitionResourceIds_Implementation() const override;
	
	// @gdemers Data Resolver for backend representation of a ATriggeringActor.
	static const TInstancedStruct<FAVVMDataResolverHelper>& GetTriggeringActorDataResolverHelper();

protected:
	UFUNCTION()
	void OnSocketParentingDeferred(AActor* Parent,
	                               AActor* Target,
	                               const FAVVMSocketTargetingDeferralContextArgs ContextArgs);
	
	UFUNCTION(Server, Reliable)
	void Server_SwapAbility(const bool bIsActive);
	
	void RegisterAbility();
	void UnRegisterAbility();

	UFUNCTION()
	void OnTriggeringAbilityClassAcquired();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Most-likely not wanted. We want to track what's in the user active hands."))
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<UTriggeringAbility> TriggeringAbilityClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="TriggeringDefinitionDataTableRow"))
	FDataRegistryId TriggeringDefinitionId = FDataRegistryId();

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMResourceManagerComponent> ResourceManagerComponent = nullptr;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayAbilitySpecHandle TriggeringAbilitySpecHandle = FGameplayAbilitySpecHandle();

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<const UAttributeSet> OwnedAttributeSet = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TSharedPtr<FStreamableHandle> TriggeringAbilityClassHandle = nullptr;
	FDelegateHandle DeferredSocketParentingDelegateHandle;

	friend class UTriggeringUtils;
};

/**
 *	Class description:
 *	
 *	UTriggeringUtils is a utility class for reusable actions. 
 */
UCLASS()
class WEAPONSAMPLE_API UTriggeringUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Weapon|Utils")
	static void Swap(AActor* UnEquip,
	                 AActor* Equip,
	                 const FAVVMSocketTargetingDeferralContextArgs& ContextArgs);
};
