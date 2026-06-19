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

#include "DataRegistryId.h"
#include "UObject/Object.h"

#include "SkillTreeUtils.generated.h"

struct FSkillTreeNodePhase;
class UGameplayEffect;

/**
 *	Class description:
 *
 *	USkillTreeUtils expose a set of utility function relevant to the skill tree system, and the
 *	backend representation.
 */
UCLASS()
class SKILLSAMPLE_API USkillTreeUtils : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FString CreateDefaultSkillTreeProviders();

	UFUNCTION(BlueprintCallable)
	static FString CreateSkillTreeProvider(const int32 ProviderId,
	                                       const TArray<int32>& NewPrivateTreeNodeIds);

	UFUNCTION(BlueprintCallable)
	static FString ModifySkillTreeProvider(const FString& NewPayload,
	                                       const int32 ProviderId,
	                                       const TArray<int32>& NewPrivateTreeNodeIds);

	UFUNCTION(BlueprintCallable)
	static TArray<FString> GetSkillTreeProviderPayloads(const FString& NewPayload);

	UFUNCTION(BlueprintCallable)
	static int32 CreateDefaultPrivateTreeNodeId(const FDataRegistryId& TreeNodeEffectRegistryId,
	                                            const int32 EffectLevel);

	UFUNCTION(BlueprintCallable)
	static FString GetSkillTreeProviderById(const FString& NewPayload,
	                                        const int32 NewProviderId);

	UFUNCTION(BlueprintCallable)
	static void GetSkillTreeProvider(const FString& NewPayload,
	                                 int32& OutProviderId,
	                                 TArray<int32>& OutPrivateTreeNodeIds);

	UFUNCTION(BlueprintCallable)
	static int32 GetSkillTreeNodePrivateId(const FString& NewPayload,
	                                       const TArray<int32>& NewPrivateIds,
	                                       const int32 PhysicalGlobalId);

	UFUNCTION(BlueprintCallable)
	static bool GetOuterSourceType(const AActor* Outer, ESkillTreeSrcType& OutSrcType);
};
