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

#include "InteractionExecutionContext.generated.h"

/**
 *	Class description:
 *
 *	FInteractionExecutionContext is a POD that hold the action to be executed during Ability Commit.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionExecutionContext
{
	GENERATED_BODY()

	virtual ~FInteractionExecutionContext() = default;

	virtual void PumpHeartbeat(const AActor* NewInstigator,
	                           const AActor* NewTarget,
	                           const float NewDelta) const PURE_VIRTUAL(PumpHeartbeat, return;);

	virtual void Execute(const AActor* NewInstigator,
	                     const AActor* NewTarget) const PURE_VIRTUAL(Execute, return;)

	virtual void Kill(const AActor* NewInstigator,
	                  const AActor* NewTarget) const PURE_VIRTUAL(Kill, return;);
};

/**
 *	Class description:
 *
 *	FInteractionExecutionContextAVVMNotify is a POD that hold the action to be executed during Ability Commit for pushing a Ctx View on screen.
 */
USTRUCT()
struct INTERACTIONSAMPLE_API FInteractionExecutionContextAVVMNotify : public FInteractionExecutionContext
{
	GENERATED_BODY()

	virtual void PumpHeartbeat(const AActor* NewInstigator,
	                           const AActor* NewTarget,
	                           const float NewDelta) const override;

	virtual void Execute(const AActor* NewInstigator,
	                     const AActor* NewTarget) const override;

	virtual void Kill(const AActor* NewInstigator,
	                  const AActor* NewTarget) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag PumpHeartbeatChannelTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag ExecuteChannelTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag KillChannelTag = FGameplayTag::EmptyTag;
};
