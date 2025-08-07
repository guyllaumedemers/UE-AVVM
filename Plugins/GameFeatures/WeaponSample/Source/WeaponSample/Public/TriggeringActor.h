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

#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"

#include "TriggeringActor.generated.h"

struct FWeaponAttachmentModifierContext;
class ATriggeringAttachmentActor;
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
class WEAPONSAMPLE_API ATriggeringActor : public AActor
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

	UFUNCTION(BlueprintCallable)
	void SpawnAndSwapAttachment(const TSubclassOf<ATriggeringAttachmentActor>& NewAttachmentClass,
	                            const FGameplayTag& NewAttachmentSlotTag);

protected:
	UFUNCTION(BlueprintCallable)
	void RegisterAbility();

	UFUNCTION(BlueprintCallable)
	void UnRegisterAbility();

	UFUNCTION()
	void OnSoftObjectAcquired();

	void GetAllAttachmentMods(FWeaponAttachmentModifierContext& OutResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ToolTip="Most-likely not wanted. We want to track what's in the user active hands."))
	bool bShouldAsyncLoadOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UTriggeringAbility> TriggeringAbilityClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UTriggeringAbility> TriggeringAbility = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ToolTip="Flag for tracking the state of the Triggering Actor so we allow swapping attachment on already occupied slot. Required=True, for editing."))
	bool bCanEditAttachments = false;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<const ATriggeringAttachmentActor>> RegisteredAttachments;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	struct FAttachmentSpawnerQueuingMechanism
	{
		// TODO @gdemers Define how swapping request are queued and executed.
	};

	TSharedPtr<FStreamableHandle> TriggeringAbilityClassHandle;
	friend class ATriggeringAttachmentActor;
};
