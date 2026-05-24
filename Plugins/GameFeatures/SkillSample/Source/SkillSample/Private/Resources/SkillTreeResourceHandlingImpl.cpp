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
#include "Resources/SkillTreeResourceHandlingImpl.h"

#include "ActorSkillTreeComponent.h"
#include "AVVMGameplayUtils.h"
#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Data/SkillTreeDefinitionDataAsset.h"
#include "GameFramework/Actor.h"

TArray<FDataRegistryId> USkillTreeResourceHandlingImpl::ProcessResources(UActorComponent* ActorComponent, const TArray<UObject*>& Resources) const
{
	auto* SkillTreeComponent = Cast<UActorSkillTreeComponent>(ActorComponent);
	if (!IsValid(SkillTreeComponent) || !UAVVMGameplayUtils::HasNetworkAuthority(SkillTreeComponent->GetTypedOuter<AActor>()))
	{
		return TArray<FDataRegistryId>{};
	}

	TArray<FDataRegistryId> OutResources;
	TArray<UObject*> OutSkillTreeNodes;
	TArray<UObject*> OutSkillTreeNodeEffects;

	for (UObject* Resource : Resources)
	{
		const auto* SkillTreeAsset = Cast<USkillTreeDefinitionDataAsset>(Resource);
		if (IsValid(SkillTreeAsset))
		{
			OutResources.Append(SkillTreeAsset->GetSkillTreeNodeObjectIds());
			continue;
		}

		const auto* SkillTreeNodeAsset = Cast<USkillTreeNodeDefinitionDataAsset>(Resource);
		if (IsValid(SkillTreeNodeAsset))
		{
			OutSkillTreeNodes.Add(Resource);
			continue;
		}

		const auto* SkillTreeNodeEffectAsset = Cast<UAVVMAbilityDefinitionDataAsset>(Resource);
		if (IsValid(SkillTreeNodeEffectAsset) && SkillTreeNodeEffectAsset->DoesInitializeViaExternalPlugin())
		{
			OutSkillTreeNodeEffects.Add(Resource);
			continue;
		}
	}

	if (!OutSkillTreeNodes.IsEmpty())
	{
		SkillTreeComponent->SetupSkillTreeNodeObjects(OutSkillTreeNodes);
	}

	if (!OutSkillTreeNodeEffects.IsEmpty())
	{
		SkillTreeComponent->SetupSkillTreeNodeEffects(OutSkillTreeNodeEffects);
	}

	return OutResources;
}
