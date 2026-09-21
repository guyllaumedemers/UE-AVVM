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
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "Data/InventoryProviderTableRow.h"
#include "Data/InventoryStubDataProviderTableRow.h"

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

UPresetLoadoutStubDataProvider::UPresetLoadoutStubDataProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (IsTemplate(RF_ClassDefaultObject))
	{
		UAVVMOnlineStubDataHelper::Static_RegisterPresetPropertyProvider(TAG_AVVMONLINE_BACKEND_STUB_PRESET_LOADOUT, GetClass());
	}
}

TMap<FGameplayTag/*Slot Tag*/, int32> UPresetLoadoutStubDataProvider::MakePropertyStubData() const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TMap<FGameplayTag, int32>{};
	}

	const auto* Row = Subsystem->GetCachedItem<FInventoryProviderTableRow>(UInventorySettings::GetStubDataProviderInventoryId());
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Stub Data Provider.")))
	{
		return TMap<FGameplayTag, int32>{};
	}

	TMap<FGameplayTag, int32> OutLoadout;
	TArray<int32> OutItems;

	UInventoryUtils::CreateInventoryProvider(Row, OutLoadout, OutItems);
	return OutLoadout;
}

UInventoryDependencyGraphStubDataProvider::UInventoryDependencyGraphStubDataProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (IsTemplate(RF_ClassDefaultObject))
	{
		UAVVMOnlineStubDataHelper::Static_RegisterPropertyProvider(TAG_AVVMONLINE_BACKEND_STUB_INVENTORY_DEPENDENCY_GRAPH, GetClass());
	}
}

TArray<int32> UInventoryDependencyGraphStubDataProvider::MakePropertyStubData() const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<int32>{};
	}

	const auto* Row = Subsystem->GetCachedItem<FStubData_InventoryStubDataProviderTableRow>(UInventorySettings::GetStubDataInventoryDependencyGraphId());
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Stub Data Provider Complex Lookup.")))
	{
		return TArray<int32>{};
	}

	// @gdemers allow assigning proper instance id to items.
	TMap<const UItemObject*/*CDO*/, int32/*Counter*/> InstanceCount{};

	TArray<int32> OutComplexDependencies{};
	for (const auto& [ItemObjectClass, ComplexDependencies] : Row->InventoryDependencyGraph)
	{
		if (ItemObjectClass.IsNull())
		{
			continue;
		}

		// TODO @gdemers Improve on this. I dont like that its synchronous.
		const UClass* ItemClass = ItemObjectClass.LoadSynchronous();
		if (!IsValid(ItemClass))
		{
			continue;
		}

		const auto* ItemObjectCDO = ItemClass->GetDefaultObject<UItemObject>();
		const int32 ItemPhysicalGlobalId = UInventoryUtils::GetObjectUniqueIdentifier(ItemObjectCDO);

		const int32 ItemVirtualGlobalId = UAVVMOnlineInventoryUtils::TranslatePhysicalAddressingDependencyGraph((1 << 2/*item bit-index*/),
		                                                                                                        ItemPhysicalGlobalId,
		                                                                                                        GET_ITEM_LOOKUP_VIRTUAL_GLOBAL_ID_BIT_RANGE,
		                                                                                                        GET_ITEM_LOOKUP_VIRTUAL_GLOBAL_ID_RSHIFT);
		
		int32& OutItemCount = InstanceCount.FindOrAdd(ItemObjectCDO);
		++OutItemCount;

		for (const auto& DependencyObjectClass : ComplexDependencies.Dependencies)
		{
			if (DependencyObjectClass.IsNull())
			{
				continue;
			}

			// TODO @gdemers Improve on this. I dont like that its synchronous.
			const UClass* DependencyClass = DependencyObjectClass.LoadSynchronous();
			if (!IsValid(DependencyClass))
			{
				continue;
			}

			const auto* AttachmentObjectCDO = DependencyClass->GetDefaultObject<UItemObject>();
			const int32 AttachmentPhysicalGlobalId = UInventoryUtils::GetObjectUniqueIdentifier(AttachmentObjectCDO);

			const int32 AttachmentVirtualGlobalId = UAVVMOnlineInventoryUtils::TranslatePhysicalAddressingDependencyGraph((1 << 0/*attachment bit-index*/),
			                                                                                                              AttachmentPhysicalGlobalId,
			                                                                                                              GET_ATTACHMENT_LOOKUP_VIRTUAL_GLOBAL_ID_BIT_RANGE,
			                                                                                                              GET_ATTACHMENT_LOOKUP_VIRTUAL_GLOBAL_ID_RSHIFT);
			
			int32& OutAttachmentCount = InstanceCount.FindOrAdd(AttachmentObjectCDO);
			++OutAttachmentCount;

			// @gdemers IMPORTANT - Both bit encoding are different. virtual address translation
			// is required to generate the proper lookup.
			const int32 DependencyBitmask = (
				ItemVirtualGlobalId +
				UAVVMOnlineEncodingUtils::EncodeInt32(OutItemCount, GET_ITEM_LOOKUP_INSTANCED_ID_BIT_RANGE, GET_ITEM_LOOKUP_INSTANCED_ID_RSHIFT) +
				AttachmentVirtualGlobalId +
				UAVVMOnlineEncodingUtils::EncodeInt32(OutAttachmentCount, GET_ATTACHMENT_LOOKUP_INSTANCED_ID_BIT_RANGE, GET_ATTACHMENT_LOOKUP_INSTANCED_ID_RSHIFT)
			);

			OutComplexDependencies.Add(DependencyBitmask);
		}
	}

	return OutComplexDependencies;
}
