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

#include "AbilitySystemComponent.h"
#include "DataRegistryId.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"

#include "ActorItemProgressionComponent.generated.h"

/**
 *	Class description:
 *
 *	UActorItemProgressionComponent is a CORE component of the inventory system for progression support. It is added
 *	dynamically at Runtime by the inventory system to a UItemObject to prevent direct reference on Actor type which may not
 *	be defined under this GFP.
 *
 *	UItemObject may refer to registry id pointing to Actor type that exist only in external GFP. Based on this restriction,
 *	the progression system dynamically add itself to the Item Actor and apply progression information to the Actor in a
 *	more flexible fashion.
 */
UCLASS(ClassGroup=("Inventory"), NotBlueprintable, NotPlaceable, meta=(BlueprintSpawnableComponent))
class INVENTORYPROGRESSIONSAMPLE_API UActorItemProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void SetProgressionIndex(const int32 NewProgressionIndex);

	UFUNCTION(BlueprintCallable)
	void RequestItemProgression(const FDataRegistryId& NewItemProgressionId);

	UFUNCTION(BlueprintCallable)
	void SetupItemProgressionStages(const TArray<UObject*>& NewResources);

protected:
	void OnItemProgressionStagesRetrieved();
	void OnProgressionStackingEffectsRetrieved();

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 CurrentProgressionStageIndex = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> ActiveGameplayEffectHandles;

	TSharedPtr<FStreamableHandle> ItemProgressionStagesHandle;
	TSharedPtr<FStreamableHandle> ItemProgressionEffectsHandle;
};
