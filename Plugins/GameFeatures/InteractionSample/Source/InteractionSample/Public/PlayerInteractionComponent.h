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

#include "GameFramework/Info.h"

#include "PlayerInteractionComponent.generated.h"

class UGameStateInteractionComponent;
class UInteraction;

/**
*	Class Description :
 *
 *	UPlayerInteractionComponent is the src component from which RPC are called from. It interface with the UGameStateInteractionComponent to record Collision Overlap events
 *	and resolve contingency between players trying to interact with replicated world actors. It exists on the APlayerController due to Unreal Owning Connection requirements to resolve RPC calls
 *	and is pushed via GFP.
 *
 *	TODO @gdemers Add support for interaction between players for cases like trying to Raise a player.
 */
UCLASS(ClassGroup=("Interaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INTERACTIONSAMPLE_API UPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInteractionComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure)
	static UPlayerInteractionComponent* GetActorComponent(const AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void AttemptRecordBeginOverlap(const AActor* NewInstigator,
	                               const AActor* NewTarget,
	                               const bool bPreventContingency);

	UFUNCTION(BlueprintCallable)
	void AttemptRecordEndOverlap(const AActor* NewInstigator,
	                             const AActor* NewTarget);

protected:
	UFUNCTION(Server, Unreliable)
	void ServerRPC_RecordBeginInteraction(const AActor* NewInstigator,
	                                      const AActor* NewTarget);

	UFUNCTION(Server, Unreliable)
	void ServerRPC_RecordEndInteraction(const AActor* NewInstigator,
	                                    const AActor* NewTarget);

	TWeakObjectPtr<UGameStateInteractionComponent> GameStateInteractionComponent = nullptr;
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
