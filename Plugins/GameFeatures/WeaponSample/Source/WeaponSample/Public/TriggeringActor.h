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
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceProvider.h"

#include "TriggeringActor.generated.h"

struct FStreamableHandle;
class UAVVMAbilitySystemComponent;
class UTriggeringAbility;

/**
 *	Class description:
 *
 *	ATriggeringActor is a triggering system that executes behaviour such as triggering or targeting.
 */
UCLASS(BlueprintType, Blueprintable)
class WEAPONSAMPLE_API ATriggeringActor : public AActor,
                                          public IAbilitySystemInterface,
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

	// @gdemers IAVVMResourceProvider
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const override;
	virtual TArray<FDataRegistryId> GetResourceDefinitionResourceIds_Implementation() const override;

protected:
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
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TSharedPtr<FStreamableHandle> TriggeringAbilityClassHandle = nullptr;
	
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
	static void Swap(ATriggeringActor* UnEquip,
	                 ATriggeringActor* Equip);
};
