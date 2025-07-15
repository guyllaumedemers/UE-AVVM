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

#include "AVVMReplicatedTagComponent.generated.h"

/**
 *	Class description:
 *
 *	UAVVMReplicatedTagComponent is a general purpose component pushed on an Actor that allow external system to
 *	compare against the current state and execute specific behavior.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMReplicatedTagComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReplicatedTagChanged, const FGameplayTagContainer&, NewTags);

public:
	UAVVMReplicatedTagComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void Append(const FGameplayTagContainer& NewTags);

	UFUNCTION(BlueprintCallable)
	void Remove(const FGameplayTagContainer& NewTags);

	UFUNCTION(BlueprintCallable)
	bool HasAnyExact(const FGameplayTagContainer& NewTags);

	UFUNCTION(BlueprintCallable)
	bool HasAllExact(const FGameplayTagContainer& NewTags);

	UPROPERTY(BlueprintAssignable)
	FOnReplicatedTagChanged OnReplicatedTagChanged;

protected:
	UFUNCTION()
	void OnRep_FlagsModified(const FGameplayTagContainer OldFlags);

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_FlagsModified")
	FGameplayTagContainer Flags = FGameplayTagContainer::EmptyContainer;
};
