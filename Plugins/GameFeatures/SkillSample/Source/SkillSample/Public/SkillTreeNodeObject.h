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

#include "ActiveGameplayEffectHandle.h"
#include "DataRegistryId.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#include "SkillTreeNodeObject.generated.h"

struct FStreamableHandle;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnRequestGameplayEffectClassComplete, const UClass*, NewActorClass, USkillTreeNodeObject*, NewSkillTreeNodeObject);

/**
 *	Class description:
 *	
 *	FSkillTreeSparseData is a Shared representation of a class object immutable data. It reduces memory footprint
 *	by removing the need to allocate that data on instanced class object, and instead reference the shared memory.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeSparseData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef, ItemStruct="AVVMAbilityDefinitionDataTableRow"))
	FDataRegistryId SkillTreeEffectId = FDataRegistryId();

	// @gdemers reference the ui definition of the referenced Effect this USkillTreeNodeObject owns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(GetByRef, ItemStruct="AVVMActorUIDefinitionDataTableRow"))
	FDataRegistryId SkillTreeEffectUIId = FDataRegistryId();
};

/**
 *	Class description:
 *	
 *	USkillTreeNodeObject is a generic UObject type that allow granting a gameplay effect to the Owning outer referencing
 *	a UActorSkillTreeComponent.
 */
UCLASS(BlueprintType, Blueprintable, SparseClassDataTypes="SkillTreeSparseData")
class SKILLSAMPLE_API USkillTreeNodeObject final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags);
	
	UFUNCTION(BlueprintCallable)
	const FDataRegistryId& BP_GetSkillTreeEffectId() const;

	UFUNCTION(BlueprintCallable)
	const FDataRegistryId& BP_GetSkillTreeEffectUIId() const;

	void GetGameplayEffectClassAsync(const UObject* NewGameplayEffectDefinitionDataAsset,
	                                 const FOnRequestGameplayEffectClassComplete& OnRequestGameplayEffectClassComplete);

	UFUNCTION(BlueprintCallable)
	void SetActiveGameplayEffectHandle(const FActiveGameplayEffectHandle& NewActiveGameplayEffectHandle);

protected:
	UFUNCTION()
	void OnGameplayEffectClassAcquired(FOnRequestGameplayEffectClassComplete Callback);

	// @gdemers : cached handle to support adding/removing effects based on user interaction.
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle = FActiveGameplayEffectHandle();

	TSharedPtr<FStreamableHandle> StreamingHandle = nullptr;

private:
	// @gdemers this flag aggregate the relevant information that defines our TreeNode. Are we a Skill, a Perk, or a Trait.
	// More importantly, are we unlocked/purchased ? What is our level requirements for unlocking, etc... 
	int32 PrivateTreeNodeId = INDEX_NONE;
	friend class USkillTreeNodeObjectUtils;
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
	                                   USkillTreeNodeObject* UnInitializedSkillTreeNodeObject);

	UFUNCTION(BlueprintCallable)
	static int32 RuntimeInitOnlineItem(const UObject* Outer,
	                                   const TArray<int32>& NewPrivateIds,
	                                   const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
	                                   USkillTreeNodeObject* UnInitializedSkillTreeNodeObject);
};
