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

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceProvider.h"

#include "TriggeringActor.generated.h"

class ATriggeringAttachmentActor;
class UAttachmentManagerComponent;
class UAVVMResourceManagerComponent;
class USkeletalMeshComponent;
class UTriggeringAbility;

/**
 *	Class description:
 *
 *	FWeaponTargetHitDataArgs is a POD struct that encapsulate relevant data to be forwarded to BP
 *	based on received targeting calculation ran by the Triggering Ability.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FWeaponTargetHitDataArgs
{
	GENERATED_BODY()

	// TODO @gdemers Define properties
};

/**
 *	Class description:
 *
 *	ATriggeringActor is a triggering system that executes behaviour such as firing or
 *	targeting. It is invoked from the referenced ability and apply instancing actors such as Vfx for ability location src (Muzzle), Impacts and Decals.
 */
UCLASS()
class WEAPONSAMPLE_API ATriggeringActor : public AActor,
                                          public IAVVMResourceProvider
{
	GENERATED_BODY()

public:
	ATriggeringActor(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Swap(const bool bIsActive);

	UFUNCTION(BlueprintImplementableEvent, meta=(ToolTip="Apply relevant vfx to available target datas."))
	void Trigger(const FWeaponTargetHitDataArgs& NewTargetHitDataArgs);

	UFUNCTION(BlueprintImplementableEvent, meta=(ToolTip="Apply markers to targeted actors."))
	void Mark(const FWeaponTargetHitDataArgs& NewTargetHitDataArgs);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SpawnAndSwapAttachment(const FDataRegistryId& NewAttachmentId);

	// @gdemers IAVVMResourceProvider
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const override;

protected:
	void RegisterAbility();
	void UnRegisterAbility();

	UFUNCTION()
	void OnSoftObjectAcquired();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(ToolTip="Most-likely not wanted. We want to track what's in the user active hands."))
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FDataRegistryId TriggeringDefinitionId = FDataRegistryId();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<UTriggeringAbility> TriggeringAbilityClass = nullptr;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMResourceManagerComponent> ResourceManagerComponent = nullptr;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttachmentManagerComponent> AttachmentManagerComponent = nullptr;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayAbilitySpecHandle TriggeringAbilitySpecHandle = FGameplayAbilitySpecHandle();

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TSharedPtr<FStreamableHandle> TriggeringAbilityClassHandle;
};
