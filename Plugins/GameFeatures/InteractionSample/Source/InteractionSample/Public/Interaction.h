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

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS
#include "UObject/Object.h"

#include "Interaction.generated.h"

/**
 *	Class Description :
 *
 *	UInteraction object is the data representation of a UPlayer collision with a replicated world actor. It captures the Target and Instigator
 *	of the collision event, and allow Server code to resolve contingency between players so only one player can produce an interaction request.
 */
UCLASS(BlueprintType)
class INTERACTIONSAMPLE_API UInteraction : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	UFUNCTION(BlueprintCallable)
	bool DoesPartialMatch(const AActor* NewInstigator) const;

	UFUNCTION(BlueprintCallable)
	bool DoesExactMatch(const AActor* NewInstigator,
	                    const AActor* NewTarget) const;

	UFUNCTION(BlueprintCallable)
	bool IsEqual(const UInteraction* Other) const;

	UFUNCTION(BlueprintCallable)
	bool CanInteract() const;

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Actor on which the collision event executed from."))
	const AActor* GetTarget() const;

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Actor that entered/exited the collision range."))
	const AActor* GetInstigator() const;

protected:
	UPROPERTY(Transient, BlueprintReadWrite, Replicated)
	TObjectPtr<const AActor> Target = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite, Replicated)
	TObjectPtr<const AActor> Instigator = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite, Replicated)
	bool bIsInteractable = true;

private:
	void operator()(const AActor* NewInstigator,
	                const AActor* NewTarget);

	friend class UActorInteractionComponent;
};
