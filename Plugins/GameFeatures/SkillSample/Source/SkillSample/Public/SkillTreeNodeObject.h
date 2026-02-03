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

#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "SkillTreeNodeObject.generated.h"

class UGameplayEffect;

/**
 *	Class description:
 *	
 *	USkillTreeNodeObject is a generic UObject type that allow granting a gameplay effect to the Owning outer referencing
 *	a UActorSkillTreeComponent.
 */
UCLASS(BlueprintType, Blueprintable)
class SKILLSAMPLE_API USkillTreeNodeObject : public UObject
{
	GENERATED_BODY()

protected:
	// @gdemers IMPORTANT : This represents whatever your Skill, Perk or Traits wants. Learn how to use it!
	// GameplayerEffect support conditional GE, and GameplayAbility granting when registered. This cover the whole case
	// for handling Skills, Perks, and Traits. No custom implementation needed!
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UGameplayEffect> GrantEffect = nullptr;

private:
	// @gdemers this flag aggregate the relevant information that defines our TreeNode. Are we a Skill, a Perk, or a Trait.
	// More importantly, are we unlocked/purchased ? What is our level requirements for unlocking, etc... 
	int32 PrivateTreeNodeId = INDEX_NONE;
};
