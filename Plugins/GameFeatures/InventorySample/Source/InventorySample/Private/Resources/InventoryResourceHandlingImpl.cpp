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
#include "Resources/InventoryResourceHandlingImpl.h"

#include "ActorInventoryComponent.h"
#include "AVVMGameplayUtils.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/ItemDefinitionDataAsset.h"

TArray<FDataRegistryId> UInventoryResourceHandlingImpl::ProcessResources(UActorComponent* ActorComponent, const TArray<UObject*>& Resources) const
{
	auto* InventoryComponent = Cast<UActorInventoryComponent>(ActorComponent);
	if (!IsValid(InventoryComponent) || !UAVVMGameplayUtils::HasNetworkAuthority(InventoryComponent->GetTypedOuter<AActor>()))
	{
		return TArray<FDataRegistryId>{};
	}

	TArray<FDataRegistryId> OutResources;
	TArray<UObject*> OutItems;
	TArray<UObject*> OutItemActors;

	for (UObject* Resource : Resources)
	{
		const auto* ItemGroup = Cast<UItemGroupDefinitionDataAsset>(Resource);
		if (IsValid(ItemGroup))
		{
			OutResources.Append(ItemGroup->GetItemIds());
			continue;
		}

		const auto* Item = Cast<UItemDefinitionDataAsset>(Resource);
		if (IsValid(Item))
		{
			OutItems.Add(Resource);
			continue;
		}

		const auto* ItemActor = Cast<UAVVMActorDefinitionDataAsset>(Resource);
		if (IsValid(ItemActor))
		{
			OutItemActors.Add(Resource);
			continue;
		}
	}

	if (!OutItems.IsEmpty())
	{
		InventoryComponent->SetupItemObjects(OutItems);
	}

	if (!OutItemActors.IsEmpty())
	{
		InventoryComponent->SetupItemActors(OutItemActors);
	}

	return OutResources;
}
