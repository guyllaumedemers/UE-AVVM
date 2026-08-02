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
#include "Resources/InventoryStubDataProvider.h"

#include "AVVMGameplayUtils.h"
#include "DataRegistrySubsystem.h"
#include "InventorySettings.h"
#include "InventoryUtils.h"
#include "IPropertyTable.h"
#include "ItemObject.h"
#include "StorageHelper.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "Data/InventoryProviderTableRow.h"

UInventoryStubDataProvider::UInventoryStubDataProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (IsTemplate(RF_ClassDefaultObject))
	{
		UAVVMOnlineStubDataHelper::Static_RegisterPropertyProvider(TAG_AVVMONLINE_BACKEND_STUB_INVENTORY, GetClass());
	}
}

TArray<int32> UInventoryStubDataProvider::MakePropertyStubData() const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<int32>{};
	}

	const auto* Row = Subsystem->GetCachedItem<FInventoryProviderTableRow>(UInventorySettings::GetStubDataProviderInventoryId());
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Stub Data Provider.")))
	{
		return TArray<int32>{};
	}

	const int32 ProviderId = UAVVMGameplayUtils::GetActorUniqueIdentifierByRegistryId(Row->InventoryProviderActorIdentifierId);
	if (!ensureAlwaysMsgf(ProviderId != INDEX_NONE,
	                      TEXT("Missing valid Id for Provider entry.")))
	{
		return TArray<int32>{};
	}

	TMap<int32, TWeakObjectPtr<const UItemObject>> ItemCDOs;
	TArray<int32> Items;

	// @gdemers generate PrivateItemIds for all entries defined for a given Provider
	for (auto& [ItemObjectClass, ProviderDefaultItemProperties] : Row->DefaultInventory)
	{
		if (ItemObjectClass.IsNull())
		{
			continue;
		}

		// TODO @gdemers Improve on this. I dont like that its synchronous.
		const UClass* Class = ItemObjectClass.LoadSynchronous();
		if (!IsValid(Class))
		{
			continue;
		}

		const auto* ItemObjectCDO = Class->GetDefaultObject<UItemObject>();
		const int32 PrivateItemId = UInventoryUtils::CreateDefaultPrivateItemId(ItemObjectCDO, ProviderDefaultItemProperties);

		ItemCDOs.FindOrAdd(PrivateItemId, ItemObjectCDO);
		Items.Add(PrivateItemId);
	}

	// @gdemers all items are initialized. if our inventory provider definition was configured correctly,
	// a valid storage object, or more are available for referencing on relevant items.
	FStorageHelper::HandleStorageAssignment(ItemCDOs, Items);
	return Items;
}
