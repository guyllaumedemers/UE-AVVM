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

#include "GameplayTagContainer.h"

#include "Iris/ReplicationState/IrisFastArraySerializer.h"

#include "GameplayTagStackContainer.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMGameplayTagStackObject
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FAVVMGameplayTagStackObject : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAVVMGameplayTagStackObject() = default;
	FAVVMGameplayTagStackObject(const FAVVMGameplayTagStackObject&) = default;
	FAVVMGameplayTagStackObject(FAVVMGameplayTagStackObject&&) noexcept = default;
	FAVVMGameplayTagStackObject& operator=(const FAVVMGameplayTagStackObject&) = default;
	FAVVMGameplayTagStackObject& operator=(FAVVMGameplayTagStackObject&&) noexcept = default;

	explicit FAVVMGameplayTagStackObject(const FGameplayTag& NewTag,
	                                     const int32 NewReplicatedStackCount,
	                                     const int32 NewPredictedStackCount);

	void PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer);
	void PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer);
	bool operator==(const FAVVMGameplayTagStackObject& Rhs) const;

protected:
	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag Tag{FGameplayTag::EmptyTag};

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 ReplicatedStackCount{INDEX_NONE};

	UPROPERTY(Transient, BlueprintReadWrite, NotReplicated)
	int32 PredictedStackCount{INDEX_NONE};
};

/**
 *	Class description:
 *	
 *	FAVVMGameplayTagStackObjectFastArray
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FAVVMGameplayTagStackObjectFastArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FIrisFastArraySerializer::FastArrayDeltaSerialize<FAVVMGameplayTagStackObject, FAVVMGameplayTagStackObjectFastArray>(GameplayTagStackObjects, DeltaParams, *this);
	}
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FAVVMGameplayTagStackObject> GameplayTagStackObjects{};

	TMap<FGameplayTag/*ComposedTag:Tag+Player*/, FAVVMGameplayTagStackObject*> FastLookup{};
};

template <>
struct TStructOpsTypeTraits<FAVVMGameplayTagStackObjectFastArray> : public TStructOpsTypeTraitsBase2<FAVVMGameplayTagStackObjectFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
