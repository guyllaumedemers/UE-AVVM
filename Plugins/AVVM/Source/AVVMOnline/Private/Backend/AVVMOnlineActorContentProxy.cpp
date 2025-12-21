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
#include "Backend/AVVMOnlineActorContentProxy.h"

#include "AVVMOnline.h"
#include "AVVMOnlineInventoryStringParser.h"

bool FAVVMItemModifierProxy::operator==(const FAVVMItemModifierProxy& Rhs) const
{
	return (ResourceId.Equals(Rhs.ResourceId));
}

bool FAVVMItemProxy::operator==(const FAVVMItemProxy& Rhs) const
{
	return (ResourceId.Equals(Rhs.ResourceId))
			&& (ModValues == Rhs.ModValues);
}

bool FAVVMItemHolderProxy::operator==(const FAVVMItemHolderProxy& Rhs) const
{
	return (ItemValues == Rhs.ItemValues);
}

bool FAVVMActorContentProxy::operator==(const FAVVMActorContentProxy& Rhs) const
{
	return (ItemHolderValues == Rhs.ItemHolderValues);
}

TArray<FDataRegistryId> UAVVMOnlineInventoryUtils::GetAllRegistryIds(const FAVVMActorContentProxy& ContentProxy)
{
	TArray<FDataRegistryId> OutResult;

	const UAVVMOnlineInventoryStringParser* InventoryParser = FAVVMOnlineModule::GetJsonParser_Inventory();
	if (!IsValid(InventoryParser))
	{
		return OutResult;
	}

	for (const FString& HolderPayload : ContentProxy.ItemHolderValues)
	{
		FAVVMItemHolderProxy OutHolderProxy;
		InventoryParser->FromString(HolderPayload, OutHolderProxy);

		// TODO @gdemers we could have a registry id that define the ItemHolder at this level.
		// To be considered later!

		for (const FString& ItemPayload : OutHolderProxy.ItemValues)
		{
			FAVVMItemProxy OutItemProxy;
			InventoryParser->FromString(ItemPayload, OutItemProxy);

			// @gdemers Add our Item RegistryId to be considered set for spawning.
			const FDataRegistryId Item_RegistryId = FDataRegistryId::ParseTypeAndName(OutItemProxy.ResourceId);
			OutResult.Add(Item_RegistryId);

			for (const FString& ItemModPayload : OutItemProxy.ModValues)
			{
				FAVVMItemModifierProxy OutItemModifierProxy;
				InventoryParser->FromString(ItemModPayload, OutItemModifierProxy);

				// @gdemers Add our Item Attachments/Mod RegistryId to be considered set for spawning.
				const FDataRegistryId ItemMod_RegistryId = FDataRegistryId::ParseTypeAndName(OutItemModifierProxy.ResourceId);
				OutResult.Add(ItemMod_RegistryId);
			}
		}
	}

	return OutResult;
}
