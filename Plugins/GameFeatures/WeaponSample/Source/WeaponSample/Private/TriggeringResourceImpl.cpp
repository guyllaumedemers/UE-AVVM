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
#include "TriggeringResourceImpl.h"

#include "AttachmentManagerComponent.h"
#include "AVVMGameplayUtils.h"
#include "ProjectileComponent.h"
#include "Data/AttachmentDefinitionDataAsset.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/ProjectileDefinitionDataAsset.h"
#include "Data/TriggeringDefinitionDataAsset.h"

TArray<FDataRegistryId> UTriggeringResourceImpl::ProcessResources(UActorComponent* ActorComponent,
                                                                  const TArray<UObject*>& Resources) const
{
	if (!IsValid(ActorComponent) || !UAVVMGameplayUtils::HasNetworkAuthority(ActorComponent->GetTypedOuter<AActor>()))
	{
		return TArray<FDataRegistryId>{};
	}

	TArray<FDataRegistryId> OutResources;
	TArray<UObject*> OutAttachmentDefinition;
	TArray<UObject*> OutAttachmentModifierDefinition;
	TArray<UObject*> OutProjectileDefinition;

	for (UObject* Resource : Resources)
	{
		const auto* TriggeringDefinition = Cast<UTriggeringDefinitionDataAsset>(Resource);
		if (IsValid(TriggeringDefinition))
		{
			OutResources.Append(TriggeringDefinition->GetDependentIds());
			continue;
		}

		const auto* AttachmentDefinition = Cast<UAttachmentDefinitionDataAsset>(Resource);
		if (IsValid(AttachmentDefinition))
		{
			OutResources.Add(AttachmentDefinition->GetTriggeringAttachmentActorId());
			continue;
		}

		const auto* AttachmentActorDefinition = Cast<UAVVMActorDefinitionDataAsset>(Resource);
		if (IsValid(AttachmentActorDefinition))
		{
			OutAttachmentDefinition.Add(Resource);
			continue;
		}

		const auto* AttachmentModifierDefinition = Cast<UAttachmentModifierDefinitionDataAsset>(Resource);
		if (IsValid(AttachmentModifierDefinition))
		{
			OutAttachmentModifierDefinition.Add(Resource);
			continue;
		}

		const auto* ProjectileDefinition = Cast<UProjectileDefinitionDataAsset>(Resource);
		if (IsValid(ProjectileDefinition))
		{
			OutProjectileDefinition.Add(Resource);
			continue;
		}
	}

	auto* AttachmentManagerComponent = Cast<UAttachmentManagerComponent>(ActorComponent);
	if (IsValid(AttachmentManagerComponent))
	{
		if (!OutAttachmentDefinition.IsEmpty())
		{
			AttachmentManagerComponent->SetupAttachments(OutAttachmentDefinition);
		}

		if (!OutAttachmentModifierDefinition.IsEmpty())
		{
			AttachmentManagerComponent->SetupAttachmentModifiers(OutAttachmentModifierDefinition);
		}
	}

	auto* ProjectileComponent = Cast<UProjectileComponent>(ActorComponent);
	if (IsValid(ProjectileComponent))
	{
		ProjectileComponent->SetupProjectiles(OutProjectileDefinition);
	}

	return OutResources;
}
