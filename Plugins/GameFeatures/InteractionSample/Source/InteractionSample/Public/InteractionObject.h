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

#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InteractionObject.generated.h"

class UActorInteractionComponent;

/**
 *	Class Description :
 *
 *	FInteractionObject is the data representation of a UPlayer collision with a replicated world actor. It captures the Target and Instigator
 *	of the collision event, and allow Server code to resolve contingency between players so only one player can produce an interaction request.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionObject : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	FInteractionObject() = default;
	FInteractionObject(const AActor* NewTarget,
	                   const AActor* NewInstigator,
	                   UActorInteractionComponent* NewInstigatorComponent);

	bool DoesPartialMatch(const AActor* NewInstigator) const;

	bool DoesExactMatch(const AActor* NewInstigator,
	                    const AActor* NewTarget) const;

	bool IsPendingKill() const;
	bool CanInteract() const;
	void Lock();
	void Unlock();
	const AActor* GetTarget() const;
	const AActor* GetInstigator() const;
	void SetPendingKill();
	
	void PreReplicatedRemove(const struct FFastArraySerializer& InArraySerializer);
	void PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer);
	void PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer);

	bool operator==(const FInteractionObject& Rhs) const;

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> Target = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> Instigator = nullptr;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UActorInteractionComponent> InstigatorComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsInteractable = true;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsPendingKill = false;
};

/**
 *	Class description:
 *	
 *	FFInteractionObjectFastArray is a FastArraySerializer derived class that pack data sequentially, and
 *	is optimized for network serialization.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInteractionObjectFastArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FIrisFastArraySerializer::FastArrayDeltaSerialize<FInteractionObject, FInteractionObjectFastArray>(InteractionObjects, DeltaParms, *this);
	}

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FInteractionObject> InteractionObjects{};
};

template <>
struct TStructOpsTypeTraits<FInteractionObjectFastArray> : public TStructOpsTypeTraitsBase2<FInteractionObjectFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
