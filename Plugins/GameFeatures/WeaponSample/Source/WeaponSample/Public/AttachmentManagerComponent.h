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
#include "Containers/Queue.h"

#include "AttachmentManagerComponent.generated.h"

class ATriggeringAttachmentActor;

/**
 *	Class description:
 *
 *	FGetAttachmentDefinitionRequestArgs is a POD type to encapsulate signature function args.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FGetAttachmentDefinitionRequestArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	FDataRegistryId AttachmentId = FDataRegistryId();

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<ATriggeringAttachmentActor> AttachmentActor = nullptr;
};

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
	TWeakObjectPtr<ATriggeringAttachmentActor> Src = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<ATriggeringAttachmentActor> Dest = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTag TargetSlotTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	UAttachmentManagerComponent is a system handling attachment equip/unequiping behaviour and applying GameplayEffects owned by the active Attachments.
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UAttachmentManagerComponent* GetActorComponent(const AActor* NewActor);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Swap(const FAttachmentSwapContextArgs& NewAttachmentSwapContext);

	void GetAttachmentDefinition(const FGetAttachmentDefinitionRequestArgs& NewRequestArgs);
	void SetupAttachmentModifiers(const TArray<UObject*>& NewResources);

protected:
	struct FAttachmentQueuingMechanism
	{
		~FAttachmentQueuingMechanism();
		void Push(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewRequest);
		TWeakObjectPtr<ATriggeringAttachmentActor> Pop();
		TQueue<TWeakObjectPtr<ATriggeringAttachmentActor>> QueuedRequest;
	};

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<ATriggeringAttachmentActor>> EquippedAttachments;

	FAttachmentQueuingMechanism QueuingMechanism;
};
