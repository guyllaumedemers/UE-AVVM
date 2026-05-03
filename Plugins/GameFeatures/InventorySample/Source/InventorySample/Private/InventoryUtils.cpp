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

#include "AVVMCharacter.h"
#include "AVVMGameplaySettings.h"
#include "AVVMGameSession.h"
#include "AVVMToolkitUtils.h"
#include "DataRegistrySubsystem.h"
#include "InventoryProvider.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "StorageHelper.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/InventoryProviderTableRow.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Tags/PrivateTags.h"

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
		TMap<FGameplayTag, int32> Loadout;
		TArray<int32> PrivateItemIds;
	};

	void ToString(const FJsonInventoryProvider& NewInventoryProvider,
	              FString& OutFormat)
	{
		TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
		JsonData->SetNumberField(TEXT("Id"), NewInventoryProvider.Id);

		TArray<TSharedPtr<FJsonValue>> Loadout;
		for (const auto& [SlotTag, PrivateItemId] : NewInventoryProvider.Loadout)
		{
			TSharedPtr<FJsonObject> KVPJsonObject = MakeShareable(new FJsonObject);
			KVPJsonObject->SetStringField(TEXT("SlotTag"), SlotTag.ToString());
			KVPJsonObject->SetNumberField("PrivateItemId", PrivateItemId);
			Loadout.Add(MakeShareable(new FJsonValueObject(KVPJsonObject)));
		}

		JsonData->SetArrayField(TEXT("Loadout"), Loadout);

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

		TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

		auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
		if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
		{
			return;
		}

		FJsonInventoryProvider InventoryProvider;
		InventoryProvider.Id = JsonData->GetIntegerField(TEXT("Id"));

		const TArray<TSharedPtr<FJsonValue>> Loadout = JsonData->GetArrayField(TEXT("Loadout"));
		for (const auto& Item : Loadout)
		{
			const TSharedPtr<FJsonObject>* OutKVPJsonObject = nullptr;
			Item->TryGetObject(OutKVPJsonObject);

			if ((OutKVPJsonObject != nullptr) && OutKVPJsonObject->IsValid())
			{
				const FString GameplayTag = (*OutKVPJsonObject)->GetStringField(TEXT("SlotTag"));
				const int32 PrivateItemId = (*OutKVPJsonObject)->GetIntegerField(TEXT("PrivateItemId"));
				InventoryProvider.Loadout.FindOrAdd(FGameplayTag::RequestGameplayTag(FName(GameplayTag)), PrivateItemId);
			}
		}

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
		!UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<const UInventoryProvider>(Outer))
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

FGameplayTag UInventoryUtils::GetItemSlotTag(const UObject* Outer,
                                             const int32 ProviderId,
                                             const int32 PrivateItemId)
{
	if (!IsValid(Outer))
	{
		return FGameplayTag::EmptyTag;
	}

	auto OutResult = FGameplayTag::EmptyTag;

	const auto* Character = Cast<AAVVMCharacter>(Outer);
	if (IsValid(Character) && Character->IsPlayerControlled())
	{
		OutResult = AAVVMGameSession::Static_GetPlayerPresetSlot(Outer->GetWorld(), ProviderId/*calling Player UniqueId*/, PrivateItemId);
	}
	else
	{
		// @gdemers we may attempt retrieving the inventory for an NPC actor
		// (or any other actor type) that are defined in backend.
		OutResult = AAVVMGameSession::Static_GetActorPresetSlot(Outer->GetWorld(), ProviderId/*calling Actor UniqueId*/, PrivateItemId);
	}

	return OutResult;
}

FGameplayTag UInventoryUtils::GetItemSlotTagFromPayload(const FString& NewPayload,
                                                        const int32 PrivateItemId)
{
	NSJsonInventory::FJsonInventoryProvider OutProvider;
	NSJsonInventory::FromString(NewPayload, OutProvider);

	const auto* SearchResult = OutProvider.Loadout.FindKey(PrivateItemId);
	if (SearchResult != nullptr)
	{
		return *SearchResult;
	}
	else
	{
		return FGameplayTag::EmptyTag;
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

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
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
		TMap<FGameplayTag, int32> Loadout;
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
			if (!IsValid(Class))
			{
				continue;
			}

			const auto* ItemObjectCDO = Class->GetDefaultObject<UItemObject>();
			const int32 PrivateItemId = UInventoryUtils::CreateDefaultPrivateItemId(ItemObjectCDO, StackCount);
			ItemCDOs.FindOrAdd(PrivateItemId, ItemObjectCDO);
			Items.Add(PrivateItemId);

			if (!Row->bCanInventoryProviderEquipItems)
			{
				continue;
			}

			const auto* SearchResult = Row->DefaultSlotTags.FindByPredicate([AllowedSlots = ItemObjectCDO->GetItemSlotTags()](const FGameplayTag& Tag)
			{
				return AllowedSlots.HasTagExact(Tag);
			});

			if ((SearchResult != nullptr) && ensureAlwaysMsgf(!Items.Contains(PrivateItemId),
			                                                  TEXT("Duplicated assignment for the current Inventory Provider.")))
			{
				Loadout.FindOrAdd(*SearchResult, PrivateItemId);
			}
		}

		// @gdemers all items are initialized. if our inventory provider definition was configured correctly,
		// a valid storage object, or more are available for referencing on relevant items.
		FStorageHelper::HandleStorageAssignment(ItemCDOs, Items);

		NSJsonInventory::FJsonInventoryProvider InventoryProvider;
		InventoryProvider.Id = ProviderId;
		InventoryProvider.Loadout = Loadout;
		InventoryProvider.PrivateItemIds = Items;

		FString OutProvider;
		NSJsonInventory::ToString(InventoryProvider, OutProvider);
		OutModifiedPayloads.Add(MakeShareable(new FJsonValueString(OutProvider)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
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
	// @gdemers retrieve unique id of item. IMPORTANT : the item should ALREADY be shifted within the Data Table that define its unique Id.
	const int32 ItemId = UInventoryUtils::GetObjectUniqueIdentifier(ItemObjectCDO);
	// @gdemers we do not assign storage as we are still unaware of which storage type is referenced
	// on the Inventory Provider we are trying to initialize. This information will be handled from within the calling function.
	return (ItemId + NewStackCount);
}
