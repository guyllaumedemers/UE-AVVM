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
#include "InventoryUtils.h"

#include "AVVMGameplaySettings.h"
#include "AVVMGameSession.h"
#include "AVVMUtils.h"
#include "DataRegistrySubsystem.h"
#include "InventoryProvider.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "NativeGameplayTags.h"
#include "StorageHelper.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/InventoryProviderTableRow.h"

/**
 *	Class description:
 *	
 *	NSJsonInventory is a private namespace for class that should be hidden from project.
 */
namespace NSJsonInventory
{
	struct FJsonInventoryProvider
	{
		int32 Id = INDEX_NONE;
		TArray<int32> PrivateItemIds;
	};

	void ToString(const FJsonInventoryProvider& NewInventoryProvider,
	              FString& OutFormat)
	{
		FJsonObject StackAllocatedFJsonObject;
		auto JsonData = TSharedPtr<FJsonObject>(&StackAllocatedFJsonObject);
		JsonData->SetNumberField(TEXT("Id"), NewInventoryProvider.Id);

		TArray<TSharedPtr<FJsonValue>> PrivateItemIds;
		for (const int32 PrivateItemId : NewInventoryProvider.PrivateItemIds)
		{
			PrivateItemIds.Add(MakeShareable(new FJsonValueNumber(PrivateItemId)));
		}

		JsonData->SetArrayField(TEXT("PrivateItemIds"), PrivateItemIds);

		FString JsonOutput;

		auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
		{
			return;
		}

		OutFormat = JsonOutput;
	}

	void FromString(const FString& NewPayload,
	                FJsonInventoryProvider& OutInventoryProvider)
	{
		if (NewPayload.IsEmpty())
		{
			return;
		}

		FJsonObject StackAllocatedFJsonObject;
		auto JsonData = TSharedPtr<FJsonObject>(&StackAllocatedFJsonObject);

		auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
		if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
		{
			return;
		}

		FJsonInventoryProvider InventoryProvider;
		InventoryProvider.Id = JsonData->GetIntegerField(TEXT("Id"));

		const TArray<TSharedPtr<FJsonValue>> PrivateItemIds = JsonData->GetArrayField(TEXT("PrivateItemIds"));
		for (const auto& PrivateItemId : PrivateItemIds)
		{
			InventoryProvider.PrivateItemIds.Add(PrivateItemId->AsNumber());
		}

		OutInventoryProvider = InventoryProvider;
	}
}

bool UInventoryUtils::GetOuterSourceType(const AActor* Outer, EItemSrcType& OutSrcType)
{
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")) ||
		!UAVVMUtils::IsBlueprintScriptInterfaceValid<const UInventoryProvider>(Outer))
	{
		return false;
	}

	OutSrcType = IInventoryProvider::Execute_GetItemSrcType(Outer);
	const bool bIsNone = EnumHasAnyFlags(OutSrcType, EItemSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IHasItemCollection::GetItemSrcType is None. Check if it was properly overriden.")))
	{
		return false;
	}

	return true;
}

int32 UInventoryUtils::DecodeItem(const int32 EncodedBits)
{
	// @gdemers our bit encoding is telling us that the element we are looking at
	// require socket support, and target our character actor.
	const bool bDoesReferenceCharacter = !!(EncodedBits & CHECK_CHARACTER_DEPENDENT_ENCODING);
	if (bDoesReferenceCharacter)
	{
		// @gdemers the above imply, we are either an attachment, or a storage.
		const int32 AttachmentId = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, GET_ATTACHMENT_ID_ENCODING_BIT_RANGE, GET_ATTACHMENT_ID_ENCODING_RSHIFT);
		if (!!AttachmentId)
		{
			return AttachmentId;
		}
		else
		{
			const int32 StorageId = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);;
			return StorageId;
		}
	}
	else
	{
		// @gdemers here, we may be bound to another actor who isn't a character, maybe ATriggeringActor (Weapon), or we are simply just an item.
		// regular Items such as weapons do not require referencing the CHECK_CHARACTER_DEPENDENT_ENCODING in their bit encoding as it's implicitly referencing one possible Actor.
		const int32 AttachmentId = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, GET_ATTACHMENT_ID_ENCODING_BIT_RANGE, GET_ATTACHMENT_ID_ENCODING_RSHIFT);
		if (!!AttachmentId)
		{
			return AttachmentId;
		}
		else
		{
			const int32 ItemId = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, GET_ITEM_ID_ENCODING_BIT_RANGE, GET_ITEM_ID_ENCODING_RSHIFT);
			return ItemId;
		}
	}
}

TArray<int32> UInventoryUtils::GetRuntimeUniqueIds(const TArray<UItemObject*>& Items)
{
	TArray<int32> OutResults;
	for (const UItemObject* Item : Items)
	{
		// @gdemers return the runtime version of our UItemObject::PrivateItemId
		// so we can compare against the version serialized with our backend.
		const int32 ItemId = UItemObjectUtils::MakeRuntimePrivateItemId(Item);
		if (ensureAlwaysMsgf(ItemId != INDEX_NONE, TEXT("Couldn't generate a valid PrivateItemId using runtime information.")))
		{
			OutResults.AddUnique(ItemId);
		}
	}

	return OutResults;
}

int32 UInventoryUtils::GetObjectUniqueIdentifier(const UItemObject* Item)
{
	if (!IsValid(Item))
	{
		return INDEX_NONE;
	}
	else
	{
		return UInventoryUtils::GetItemActorUniqueIdentifier({UAVVMGameplaySettings::GetActorIdentifierRegistryType(), Item->GetItemActorId().ItemName});
	}
}

int32 UInventoryUtils::GetItemActorUniqueIdentifier(const FDataRegistryId& ItemActorId)
{
	if (!ensureAlwaysMsgf(ItemActorId.IsValid(),
	                      TEXT("Composed RegistryId isn't valid. You may be missing a reference in DeveloperSettings for the Actor Identifier RegistryType.")))
	{
		return INDEX_NONE;
	}

	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return INDEX_NONE;
	}

	// @gdemers imply we pre-cache our DT (which is fine! we can set that in editor, and is lightweight)
	const auto* RowValue = Subsystem->GetCachedItem<FAVVMActorIdentifierDataTableRow>(ItemActorId);
	if (ensureAlwaysMsgf(RowValue != nullptr,
	                     TEXT("Invalid Row Entry. Make sure FAVVMActorIdentifierDataTableRow match the Data Table.")))
	{
		return RowValue->UniqueId;
	}
	else
	{
		return INDEX_NONE;
	}
}

TArray<FString> UInventoryUtils::GetInventoryProviderPayloads(const FString& NewPayload)
{
	FJsonObject StackAllocatedFJsonObject;
	auto JsonData = TSharedPtr<FJsonObject>(&StackAllocatedFJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return TArray<FString>();
	}

	TArray<FString> OutProviders;

	const TArray<TSharedPtr<FJsonValue>> InventoryProviders = JsonData->GetArrayField(TEXT("InventoryProviders"));
	for (const auto& InventoryProvider : InventoryProviders)
	{
		OutProviders.Add(InventoryProvider->AsString());
	}

	return OutProviders;
}

FString UInventoryUtils::GetInventoryProviderById(const FString& NewPayload,
                                                  const int32 NewProviderId)
{
	const TArray<FString> InventoryProviders = GetInventoryProviderPayloads(NewPayload);
	if (InventoryProviders.IsEmpty())
	{
		return FString();
	}

	const FString* SearchResult = InventoryProviders.FindByPredicate([SearchId = NewProviderId](const FString& Payload)
	{
		NSJsonInventory::FJsonInventoryProvider OutProvider;
		NSJsonInventory::FromString(Payload, OutProvider);
		return (false == (OutProvider.Id ^ SearchId));
	});

	if (SearchResult != nullptr)
	{
		return *SearchResult;
	}
	else
	{
		return TEXT("");
	}
}

int32 UInventoryUtils::GetItemPrivateId(const FString& NewPayload,
                                        const TArray<int32>& NewPrivateIds,
                                        const int32 ItemId)
{
	NSJsonInventory::FJsonInventoryProvider OutProvider;
	NSJsonInventory::FromString(NewPayload, OutProvider);

	TArray<int32> FilteredSet = OutProvider.PrivateItemIds;
	for (const int32 PrivateId : NewPrivateIds)
	{
		FilteredSet.Remove(PrivateId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = ItemId](const int32 Value)
	{
		// @gdemers decode Value (PrivateItemId) of the disk representation of the item, and parse it's type, returning an output value
		// that respect our initial bit encoding defined under AVVMOnlineInventory.h
		const int32 OutValue = UInventoryUtils::DecodeItem(Value);
		return (false == (OutValue ^ SearchId))/*if both bits are identical, return 0.*/;
	});

	if (SearchResult != nullptr)
	{
		return *SearchResult;
	}
	else
	{
		return INDEX_NONE;
	}
}

FString UInventoryUtils::ModifyInventoryProvider(const FString& NewPayload,
                                                 const int32 ProviderId,
                                                 const TArray<int32>& NewPrivateIds)
{
	TArray<NSJsonInventory::FJsonInventoryProvider> InventoryProviders;
	for (const FString& Payload : GetInventoryProviderPayloads(NewPayload))
	{
		NSJsonInventory::FJsonInventoryProvider OutProvider;
		NSJsonInventory::FromString(Payload, OutProvider);

		InventoryProviders.Add(OutProvider);
	}

	auto* SearchResult = InventoryProviders.FindByPredicate([SearchId = ProviderId](const NSJsonInventory::FJsonInventoryProvider& Provider)
	{
		return (false == (Provider.Id ^ SearchId));
	});

	if (SearchResult != nullptr)
	{
		// @gdemers overwrite all item entries within this provider.
		SearchResult->PrivateItemIds = NewPrivateIds;
	}

	TArray<TSharedPtr<FJsonValue>> OutModifiedPayloads;
	for (const auto& ModifiedProvider : InventoryProviders)
	{
		FString OutFormat;
		NSJsonInventory::ToString(ModifiedProvider, OutFormat);
		OutModifiedPayloads.Add(MakeShareable(new FJsonValueString(OutFormat)));
	}

	FJsonObject StackAllocatedFJsonObject;
	auto JsonData = TSharedPtr<FJsonObject>(&StackAllocatedFJsonObject);
	JsonData->SetArrayField(TEXT("InventoryProviders"), OutModifiedPayloads);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return FString();
	}
	else
	{
		return JsonOutput;
	}
}

FString UInventoryUtils::CreateDefaultInventoryProviders()
{
	const TSoftObjectPtr<UDataTable>& ProviderDataTable = UInventorySettings::GetDefaultProviderInventories();
	if (!ensureAlwaysMsgf(!ProviderDataTable.IsNull(),
	                      TEXT("Project doesn't reference a valid Data Table to initialize the Provider Inventories on Disk.")))
	{
		return FString();
	}

	// TODO @gdemers Improve on this. I dont like that its synchronous.
	const UDataTable* DataTable = ProviderDataTable.LoadSynchronous();
	if (!IsValid(DataTable))
	{
		return FString();
	}

	TArray<FInventoryProviderTableRow*> OutRows;
	DataTable->GetAllRows<FInventoryProviderTableRow>(TEXT(""), OutRows);

	TArray<TSharedPtr<FJsonValue>> OutModifiedPayloads;
	for (const FInventoryProviderTableRow* Row : OutRows)
	{
		if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Row entry.")))
		{
			continue;
		}

		const int32 ProviderId = UInventoryUtils::GetItemActorUniqueIdentifier(Row->InventoryProviderActorIdentifierId);
		if (!ensureAlwaysMsgf(ProviderId != INDEX_NONE,
		                      TEXT("Missing valid Id for Provider entry.")))
		{
			continue;
		}

		TMap<int32, TWeakObjectPtr<const UItemObject>> ItemCDOs;
		TArray<int32> Items;
		// @gdemers IMPORTANT : Understand that content defined within the inventory of a provider fed by DataAsset
		// are Blueprints, not elements dynamically composed. A complex entry is configured based on an Item Actor definition.
		for (auto& [ItemObjectClass, StackCount] : Row->DefaultInventory)
		{
			if (ItemObjectClass.IsNull())
			{
				continue;
			}

			// TODO @gdemers Improve on this. I dont like that its synchronous.
			const UClass* Class = ItemObjectClass.LoadSynchronous();
			if (IsValid(Class))
			{
				const auto* ItemObjectCDO = Class->GetDefaultObject<UItemObject>();
				const int32 PrivateItemId = UInventoryUtils::CreateDefaultPrivateItemId(ItemObjectCDO, StackCount);
				ItemCDOs.FindOrAdd(PrivateItemId, ItemObjectCDO);
				Items.Add(PrivateItemId);
			}
		}

		// @gdemers all items are initialized. if our inventory provider definition was configured correctly,
		// a valid storage object, or more are available for referencing on relevant items.
		FStorageHelper::HandleStorageAssignment(ItemCDOs, Items);

		NSJsonInventory::FJsonInventoryProvider InventoryProvider;
		InventoryProvider.Id = ProviderId;
		InventoryProvider.PrivateItemIds = Items;

		FString OutProvider;
		NSJsonInventory::ToString(InventoryProvider, OutProvider);
		OutModifiedPayloads.Add(MakeShareable(new FJsonValueString(OutProvider)));
	}

	FJsonObject StackAllocatedFJsonObject;
	auto JsonData = TSharedPtr<FJsonObject>(&StackAllocatedFJsonObject);
	JsonData->SetArrayField(TEXT("InventoryProviders"), OutModifiedPayloads);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return FString();
	}
	else
	{
		return JsonOutput;
	}
}

int32 UInventoryUtils::CreateDefaultPrivateItemId(const UItemObject* ItemObjectCDO,
                                                  const int32 StackCount)
{
	if (!IsValid(ItemObjectCDO))
	{
		return INDEX_NONE;
	}

	const int32 NewStackCount = UAVVMOnlineEncodingUtils::EncodeInt32(StackCount, GET_ITEM_COUNT_ENCODING_BIT_RANGE, GET_ITEM_COUNT_ENCODING_RSHIFT);
	// @gdemers retrieve unique id of item. IMPORTANT : our item id isnt shifted. we may be an attachment, or storage.
	int32 ItemId = UInventoryUtils::GetObjectUniqueIdentifier(ItemObjectCDO);

	static const auto ItemAttachmentType = FGameplayTagContainer(UInventorySettings::GetAttachmentTypeTag());
	static const auto ItemStorageType = FGameplayTagContainer(UInventorySettings::GetStorageTypeTag());

	if (ItemObjectCDO->DoesTypeHasPartialMatch(ItemStorageType))
	{
		ItemId = UAVVMOnlineEncodingUtils::EncodeInt32(ItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT) + CHECK_CHARACTER_DEPENDENT_ENCODING;
		return (ItemId + NewStackCount);
	}
	else if (ItemObjectCDO->DoesTypeHasPartialMatch(ItemAttachmentType))
	{
		ItemId = UAVVMOnlineEncodingUtils::EncodeInt32(ItemId, GET_ATTACHMENT_ID_ENCODING_BIT_RANGE, GET_ATTACHMENT_ID_ENCODING_RSHIFT);
	}

	// @gdemers we do not assign storage as we are still unaware of which storage type is referenced
	// on the Inventory Provider we are trying to initialize. This information will be handled from within the calling function.
	return (ItemId + NewStackCount);
}
