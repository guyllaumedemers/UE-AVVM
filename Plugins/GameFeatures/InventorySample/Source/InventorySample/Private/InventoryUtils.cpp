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
#include "AVVMUtils.h"
#include "InventoryProvider.h"
#include "ItemObject.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorIdentifierTableRow.h"

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
		if (IsValid(Item))
		{
			// @gdemers return the runtime version of our UItemObject::PrivateItemId so we can compare against
			// the version serialized with our backend.
			const int32 ItemId = UItemObjectUtils::MakeRuntimePrivateItemId(Item);
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

	const TSoftObjectPtr<UDataTable>& ActorIdentifierDataTable = UAVVMGameplaySettings::GetActorIdentifierDataTable();
	if (ActorIdentifierDataTable.IsNull())
	{
		return INDEX_NONE;
	}

	// @gdemers this should be fairly quick to load and not create any hitches during gameplay.
	const UDataTable* DataTable = ActorIdentifierDataTable.LoadSynchronous();
	if (!IsValid(DataTable))
	{
		return INDEX_NONE;
	}

	const auto* RowValue = DataTable->FindRow<FAVVMActorIdentifierDataTableRow>(Item->GetItemActorId().ItemName, TEXT(""));
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
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

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
	
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	const FString* SearchResult = InventoryProviders.FindByPredicate([JsonData, NewProviderId](const FString& Payload)
	{
		if (!JsonData.IsValid() || Payload.IsEmpty())
		{
			return false;
		}

		// TODO @gdemers Must ensure data is overwritten between assignment.
		TSharedPtr<FJsonObject> NewJsonData = JsonData;

		auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(Payload);
		if (!FJsonSerializer::Deserialize(JsonReaderRef, NewJsonData))
		{
			return false;
		}

		const int32 InventoryProviderId = NewJsonData->GetIntegerField(TEXT("InventoryProviderId"));
		return (false == (InventoryProviderId ^ NewProviderId));
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

TArray<FString> UInventoryUtils::GetInventoryProviderItems(const FString& NewProviderPayload)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewProviderPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return TArray<FString>();
	}

	TArray<FString> OutItems;

	const TArray<TSharedPtr<FJsonValue>> Items = JsonData->GetArrayField(TEXT("Items"));
	for (const auto& Item : Items)
	{
		OutItems.Add(Item->AsString());
	}
	
	return OutItems;
}

FString UInventoryUtils::GetInventoryProviderItemById(const FString& NewProviderPayload,
                                                      const TArray<int32>& NewPrivateIds,
                                                      const int32 NewItemId)
{
	const TArray<FString> Items = GetInventoryProviderItems(NewProviderPayload);
	if (Items.IsEmpty())
	{
		return FString();
	}
	
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	const FString* SearchResult = Items.FindByPredicate([JsonData, NewItemId, NewPrivateIds](const FString& Payload)
	{
		if (!JsonData.IsValid() || Payload.IsEmpty())
		{
			return false;
		}

		// TODO @gdemers Must ensure data is overwritten between assignment.
		TSharedPtr<FJsonObject> NewJsonData = JsonData;

		auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(Payload);
		if (!FJsonSerializer::Deserialize(JsonReaderRef, NewJsonData))
		{
			return false;
		}

		// @gdemers Filter out entries that are already assigned in the runtime representation
		// of the inventory.
		const int32 ItemId = NewJsonData->GetIntegerField(TEXT("ItemId"));
		if (NewPrivateIds.Contains(ItemId))
		{
			return false;
		}
		else
		{
			return (false == (ItemId ^ NewItemId));
		}
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

int32 UInventoryUtils::GetItemPrivateId(const FString& NewItemPayload)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewItemPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return INDEX_NONE;
	}

	const int32 PrivateId = JsonData->GetIntegerField(TEXT("PrivateId"));
	return PrivateId;
}
