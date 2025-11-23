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
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"

#include "AttachmentManagerComponent.generated.h"

class ATriggeringAttachmentActor;
struct FStreamableHandle;

/**
 *	Class description:
 *
 *	FAttachmentSwapContextArgs is a POD type to encapsulate signature function args.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FAttachmentSwapContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<ATriggeringAttachmentActor> Attachment = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTag TargetSlotTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	FAttachmentToken describe a unique identifier that increments only when default construct. Can be safely
 *	passed by copy around.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FAttachmentToken
{
	GENERATED_BODY()

	explicit FAttachmentToken()
	{
		static uint32 GlobalUniqueId = 0;
		UniqueId = ++GlobalUniqueId;
	}

	UPROPERTY()
	uint32 UniqueId = 0;
};

/**
 *	Class description:
 *
 *	FAttachmentModifierToken describe a unique identifier that increments only when default construct. Can be safely
 *	passed by copy around.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FAttachmentModifierToken
{
	GENERATED_BODY()

	explicit FAttachmentModifierToken()
	{
		static uint32 GlobalUniqueId = 0;
		UniqueId = ++GlobalUniqueId;
	}

	UPROPERTY()
	uint32 UniqueId = 0;
};

/**
 *	Class description:
 *
 *	UAttachmentManagerComponent is a system handling attachment equip/unequiping behaviour and applying GameplayEffects owned by the active Attachments.
 */
UCLASS(ClassGroup=("Weapon"), Blueprintable, meta=(BlueprintSpawnableComponent))
class WEAPONSAMPLE_API UAttachmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Swap(const FAttachmentSwapContextArgs& NewAttachmentSwapContext);

	void GetAttachmentModifierDefinition(const FDataRegistryId& NewAttachmentModifierDefinitionId) const;
	void SetupAttachments(const TArray<UObject*>& NewResources);
	void SetupAttachmentModifiers(const TArray<UObject*>& NewResources);

protected:
	UFUNCTION()
	void OnAttachmentActorRetrieved(FAttachmentToken AttachmentToken);

	UFUNCTION()
	void OnAttachmentModifiersRetrieved(FAttachmentModifierToken AttachmentModifierToken);

	// @gdemers POD type that queue data registry request.
	struct FAttachmentQueuingMechanism
	{
		FAttachmentQueuingMechanism() = default;
		~FAttachmentQueuingMechanism();

		void Push(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewAttachment);
		TWeakObjectPtr<ATriggeringAttachmentActor> PeekAtIndex(const int32 NewIndex);
		void TryExecuteNext(const UAttachmentManagerComponent* AttachmentManagerComponent);
		bool IsEmpty() const;

		TArray<TWeakObjectPtr<ATriggeringAttachmentActor>> QueuedRequest;
	};

	// @gdemers POD type that aggregate actor to be destroyed. Actors should be kept, deactivated until batching happens so if a swap
	// action occurs, we can pull out the actor and prevent allocation in world of a new actor.
	struct FAttachmentBatchingMechanism
	{
		~FAttachmentBatchingMechanism();

		void PushPendingDestroy(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewAttachment);
		void BatchDestroy();

		TArray<TWeakObjectPtr<ATriggeringAttachmentActor>> PendingDestroy;
	};

	// @gdemers property updated on the server only, theres no point in replicating this on the client as swapping, attach/detach will be request process server-side ALWAYS!
	UPROPERTY(Transient, BlueprintReadOnly, meta=(ToolTip="GameplayTagContainer that define the state of the Outer Actor. Example : InTutorial, Pre-BossFight-X, etc..."))
	FGameplayTagContainer ComponentStateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<ATriggeringAttachmentActor>> EquippedAttachments;

	TMap<uint32, TSharedPtr<FStreamableHandle>> AttachmentHandleSystem;
	TMap<uint32, TSharedPtr<FStreamableHandle>> AttachmentModifierHandleSystem;
	TSharedPtr<FAttachmentBatchingMechanism> BatchingMechanism;
	TSharedPtr<FAttachmentQueuingMechanism> QueueingMechanism;
};
