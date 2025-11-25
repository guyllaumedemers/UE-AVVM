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
#include "Components/ActorComponent.h"

#include "ActorFenceComponent.generated.h"

class UAVVMReplicatedTagComponent;

/**
 *	Class description:
 *
 *	UActorFenceComponent is a system component that compare tags against it's owning outer. It should be used
 *	during the loading phase of a world, or during travel and help determine when it's safe to execute specific actions.
 *
 *	Example : lowering loading screen when all players are ready.
 */
UCLASS(ClassGroup=("Fence"), Blueprintable, meta=(BlueprintSpawnableComponent))
class FENCINGSAMPLE_API UActorFenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorFenceComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	void TryRaise();
	void TryLower();
	
	UFUNCTION()
	void OnReplicatedTagChanged(const FGameplayTagContainer& NewTags);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Execute() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer FenceRequirements = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UAVVMReplicatedTagComponent> ReplicatedTagComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

private:
	// @gdemers WARNING! Cheats Only, should never be used elsewhere!
	void ForceLowering() const;

	friend class UFenceCheatExtension;
};
