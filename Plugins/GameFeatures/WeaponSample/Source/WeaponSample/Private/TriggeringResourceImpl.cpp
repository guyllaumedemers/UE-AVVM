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
#include "AVVMReplicatedTagComponent.h"
#include "AVVMToolkitUtils.h"
#include "ProjectileComponent.h"
#include "Components/ActorComponent.h"
#include "Data/AttachmentDefinitionDataAsset.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/TriggeringDefinitionDataAsset.h"
#include "GameFramework/Actor.h"

TArray<FDataRegistryId> UTriggeringResourceImpl::ProcessResources(UActorComponent* ActorComponent,
                                                                  const TArray<UObject*>& Resources) const
{
	auto* AttachmentManagerComponent = Cast<UAttachmentManagerComponent>(ActorComponent);
	if (!ensureAlwaysMsgf(IsValid(AttachmentManagerComponent), TEXT("Component invalid.")) ||
		!UAVVMToolkitUtils::HasNetworkAuthority(AttachmentManagerComponent->GetTypedOuter<AActor>()))
	{
		return TArray<FDataRegistryId>{};
	}

	TArray<FDataRegistryId> OutResources;
	TArray<UObject*> OutAttachmentDefinition;

	for (UObject* Resource : Resources)
	{
		const auto* TriggeringDefinition = Cast<UTriggeringDefinitionDataAsset>(Resource);
		if (IsValid(TriggeringDefinition))
		{
			OutResources.Append(TriggeringDefinition->GetAttachmentIds());

			const FGameplayTagContainer& DefaultTriggeringModeTags = TriggeringDefinition->GetDefaultTriggeringModeTags();
			if (!ensureAlwaysMsgf(DefaultTriggeringModeTags.IsValid(),
			                      TEXT("Triggering modes not supported. Check your Tags configuration!")))
			{
				continue;
			}

			// @gdemers based on the definition of the TriggeringActor Class. We have defined a set of supported tags that prevent incompatible Triggering Mode
			// being initialized with the actor representation.
			auto* ReplicatedTagComponent = UAVVMReplicatedTagComponent::Static_GetActorComponent(AttachmentManagerComponent->GetTypedOuter<AActor>());
			if (IsValid(ReplicatedTagComponent) && ensureAlwaysMsgf(ReplicatedTagComponent->HasAnyExactFilteredTags(DefaultTriggeringModeTags),
			                                                        TEXT("Triggering Actor doesnt support all Modes defined in this tag container.")))
			{
				ReplicatedTagComponent->ModifyRuntimeTags(DefaultTriggeringModeTags, {});
			}

			continue;
		}

		const auto* AttachmentDefinition = Cast<UAttachmentDefinitionDataAsset>(Resource);
		if (IsValid(AttachmentDefinition))
		{
			OutResources.Add(AttachmentDefinition->GetAttachmentActorId());
			continue;
		}

		const auto* AttachmentActorDefinition = Cast<UAVVMActorDefinitionDataAsset>(Resource);
		if (IsValid(AttachmentActorDefinition))
		{
			OutAttachmentDefinition.Add(Resource);
			continue;
		}
	}

	if (!OutAttachmentDefinition.IsEmpty())
	{
		AttachmentManagerComponent->SetupAttachments(OutAttachmentDefinition);
	}

	return OutResources;
}
