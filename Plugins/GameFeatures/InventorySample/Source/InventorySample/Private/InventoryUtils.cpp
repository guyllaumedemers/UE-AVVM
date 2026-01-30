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

#include "AVVMUtils.h"
#include "DataRegistrySubsystem.h"
#include "InventoryProvider.h"
#include "InventorySettings.h"
#include "ItemObject.h"
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
		return UInventoryUtils::GetItemActorUniqueIdentifier(Item->GetItemActorId());
	}
}

int32 UInventoryUtils::GetItemActorUniqueIdentifier(const FDataRegistryId& ItemActorId)
{
	if (!ItemActorId.IsValid())
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

FString UInventoryUtils::ModifyProfile(const UObject* WorldContextObject,
                                       const int32 ProfileId,
                                       const TArray<int32>& NewItems)
{
	// TODO @gdemers Update profile string.
	return FString();
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
		// @gdemers retrieve only the bits specific to the item id, without storage information, stacking, etc...
		const int32 FilteredId = (SearchId & Value);
		return (false == (SearchId ^ FilteredId));
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

		const int32 ProviderId = UInventoryUtils::GetItemActorUniqueIdentifier(Row->InventoryProviderActorId);
		if (!ensureAlwaysMsgf(ProviderId != INDEX_NONE,
		                      TEXT("Missing valid Id for Provider entry.")))
		{
			continue;
		}

		TArray<int32> Items;
		for (auto& [ItemObjectClass, PrivateItemIdComposition] : Row->DefaultInventory)
		{
			if (ItemObjectClass.IsNull())
			{
				continue;
			}

			// TODO @gdemers Improve on this. I dont like that its synchronous.
			const UClass* Class = ItemObjectClass.LoadSynchronous();
			if (IsValid(Class))
			{
				const int32 PrivateItemId = UInventoryUtils::CreateDefaultPrivateItemId(Class->GetDefaultObject<UItemObject>(), PrivateItemIdComposition);
				Items.Add(PrivateItemId);
			}
		}

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
                                                  const FPrivateItemIdComposition& ItemComposition)
{
	if (!IsValid(ItemObjectCDO) || ItemComposition.OwningOuter.IsNull())
	{
		return INDEX_NONE;
	}

	// TODO @gdemers Improve on this. I dont like that its synchronous.
	const UClass* Class = ItemComposition.OwningOuter.LoadSynchronous();
	if (!IsValid(Class))
	{
		return INDEX_NONE;
	}

	// @gdemers retrieve unique id of item.
	int32 ItemId = UInventoryUtils::GetObjectUniqueIdentifier(ItemObjectCDO);
	int32 AttachmentId = NULL;

	// @gdemers if valid, this implies we are initializing an attachment, and referencing the object it's attached to.
	const UItemObject* DependentOnItemObjectCDO = Class->GetDefaultObject<UItemObject>();
	if (IsValid(DependentOnItemObjectCDO))
	{
		AttachmentId = ItemId + (1 << GET_ATTACHMENT_ID_ENCODING_RSHIFT);
		ItemId = UInventoryUtils::GetObjectUniqueIdentifier(DependentOnItemObjectCDO);
	}

	const int32 StackCount = ItemComposition.DefaultStackCount + (1 << GET_ITEM_COUNT_ENCODING_RSHIFT);

	// TODO @gdemers define how storage referencing must be handled here!
	const int32 StorageId = 1 + (1 << GET_STORAGE_ID_ENCODING_RSHIFT);
	const int32 StoragePosition = 1 + (1 << GET_ITEM_POSITION_ENCODING_RSHIFT);

	return (ItemId + StackCount + StorageId + StoragePosition + AttachmentId);
}
