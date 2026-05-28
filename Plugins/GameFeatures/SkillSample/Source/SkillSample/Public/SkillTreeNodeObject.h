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
#include "StructUtils/InstancedStruct.h"

#include "SkillTreeNodeObject.generated.h"

class UGameplayEffect;
struct FStreamableHandle;

/**
 *	Class description:
 *	
 *	FSkillTreeNodeObject is a generic type that reference the handle of a gameplay effect granted
 *	to the Owning UActorSkillTreeComponent.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeNodeObject : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FSkillTreeNodeObject() = default;
	FSkillTreeNodeObject(const int32 NewPrivateTreeNodeId,
	                     const int32 NewActiveGameplayEffectHandleTypeHash);
	
	const int32 GetActiveEffectHandleTypeHash() const;
	const int32 GetSkillTreeNodePrivateId() const;

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ActiveGameplayEffectHandleTypeHash = INDEX_NONE;

	// @gdemers this flag aggregate the relevant information that defines our TreeNode. Are we a Skill, a Perk, or a Trait.
	// More importantly, are we unlocked/purchased ? What is our level requirements for unlocking, etc... 
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 PrivateTreeNodeId = INDEX_NONE;
	
	friend class USkillTreeNodeObjectUtils;
};

/**
 *	Class description:
 *	
 *	FSkillTreeNodeObjectFastArray is a FastArraySerializer derived class that pack data sequentially, and
 *	is optimized for network serialization.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeNodeObjectFastArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FIrisFastArraySerializer::FastArrayDeltaSerialize<FSkillTreeNodeObject, FSkillTreeNodeObjectFastArray>(SkillTreeNodeObjects, DeltaParms, *this);
	}

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FSkillTreeNodeObject> SkillTreeNodeObjects;
};

template <>
struct TStructOpsTypeTraits<FSkillTreeNodeObjectFastArray> : public TStructOpsTypeTraitsBase2<FSkillTreeNodeObjectFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/**
 *	Class description:
 *	
 *	USkillTreeNodeObjectUtils is a blueprint function library that expose reusable api.
 */
UCLASS()
class SKILLSAMPLE_API USkillTreeNodeObjectUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static int32 RuntimeInitStaticItem(const UObject* Outer,
	                                   const TArray<int32>& NewPrivateIds,
	                                   const UGameplayEffect* SkillTreeNodeEffectCDO);

	UFUNCTION(BlueprintCallable)
	static int32 RuntimeInitOnlineItem(const UObject* Outer,
	                                   const TArray<int32>& NewPrivateIds,
	                                   const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
	                                   const UGameplayEffect* SkillTreeNodeEffectCDO);
	
	UFUNCTION(BlueprintCallable)
	static int32 FilterTreeNodePrivateId(const int32 EncodedBits);
	
	UFUNCTION(BlueprintCallable)
	static bool IsAttachment(const int32 EncodedBits);
	
	UFUNCTION(BlueprintCallable)
	static bool IsWeapon(const int32 EncodedBits);
};
