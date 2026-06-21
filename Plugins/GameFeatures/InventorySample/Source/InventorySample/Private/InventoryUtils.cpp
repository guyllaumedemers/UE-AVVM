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
#include "AVVMGameplayUtils.h"
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
			KVPJsonObject->SetNumberField(TEXT("PrivateItemId"), PrivateItemId);
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

		const int32 ProviderId = UAVVMGameplayUtils::GetActorUniqueIdentifierByRegistryId(Row->InventoryProviderActorIdentifierId);
		if (!ensureAlwaysMsgf(ProviderId != INDEX_NONE,
		                      TEXT("Missing valid Id for Provider entry.")))
		{
			continue;
		}

		TMap<int32, TWeakObjectPtr<const UItemObject>> ItemCDOs;
		TMap<FGameplayTag, int32> Loadout;
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

			if (!Row->bCanInventoryProviderEquipItems)
			{
				continue;
			}

			const auto* SearchResult = Row->DefaultSlotTags.FindByPredicate([AllowedSlots = ItemObjectCDO->BP_GetItemSlotTags()](const FGameplayTag& Tag)
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

		const FString OutProvider = UInventoryUtils::CreateInventoryProvider(ProviderId, Loadout, Items);
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

FString UInventoryUtils::CreateInventoryProvider(const int32 ProviderId,
                                                 const TMap<FGameplayTag, int32>& Loadout,
                                                 const TArray<int32>& PrivateItemIds)
{
	NSJsonInventory::FJsonInventoryProvider InventoryProvider;
	InventoryProvider.Id = ProviderId;
	InventoryProvider.Loadout = Loadout;
	InventoryProvider.PrivateItemIds = PrivateItemIds;

	FString OutProvider;
	NSJsonInventory::ToString(InventoryProvider, OutProvider);

	return OutProvider;
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

void UInventoryUtils::GetInventoryProvider(const FString& NewPayload,
                                           int32& OutProviderId,
                                           TMap<FGameplayTag, int32>& OutLoadout,
                                           TArray<int32>& OutPrivateItemIds)
{
	NSJsonInventory::FJsonInventoryProvider OutProvider;
	NSJsonInventory::FromString(NewPayload, OutProvider);

	OutProviderId = OutProvider.Id;
	OutLoadout = OutProvider.Loadout;
	OutPrivateItemIds = OutProvider.PrivateItemIds;
}

int32 UInventoryUtils::CreateDefaultPrivateItemId(const UItemObject* ItemObjectCDO,
                                                  const FProviderDefaultItemProperties& ProviderItemProperties)
{
	if (!IsValid(ItemObjectCDO))
	{
		return INDEX_NONE;
	}

	// @gdemers Relationship bitmask define dependency on another element (example :
	// an attachment being dependent on a character, or weapon). 
	const int32 RelationshipBitMask = ProviderItemProperties.GetRelationshipBitmask();
	const int32 PhysicalGlobalId = UInventoryUtils::GetObjectUniqueIdentifier(ItemObjectCDO);
	const int32 VirtualGlobalId = UInventoryUtils::TranslatePhysicalAddressing(RelationshipBitMask, PhysicalGlobalId);

	const int32 InstancedId = UAVVMOnlineEncodingUtils::EncodeInt32(ProviderItemProperties.InstancedId, GET_ELEMENT_INSTANCED_ID_BIT_RANGE, GET_ELEMENT_INSTANCED_ID_RSHIFT);

	int32 VirtualStorageId = 0;
	if (false == !!RelationshipBitMask/*storage, or 000 bitmask*/)
	{
		const int32 BaseId = (PhysicalGlobalId - GET_STORAGE_PHYSICAL_ADDRESSING_OFFSET);
		// @gdemers IMPORTANT : Remember that Storage are valid UItemObject references (ex: a backpack, belt-pouch, etc...)
		// Our Storage Virtual Global Id encoded within the PrivateItemId will simply be duplicated into the range defined for storage.
		VirtualStorageId = UAVVMOnlineEncodingUtils::EncodeInt32(BaseId, GET_STORAGE_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_STORAGE_VIRTUAL_GLOBAL_ID_RSHIFT);
	}

	const int32 StackCount = UAVVMOnlineEncodingUtils::EncodeInt32(ProviderItemProperties.StackCount, GET_ELEMENT_STACK_COUNT_BIT_RANGE, GET_ELEMENT_STACK_COUNT_RSHIFT);
	// @gdemers StoragePosition is managed by the calling function, and require all PrivateItemId to be
	// created first. Doing so, a storage Item will have been created, based on system requirements, and used during the binding process
	// to set storage positions for each objects defined in the inventory system of this provider actor.
	return (RelationshipBitMask
		+ VirtualGlobalId
		+ InstancedId
		+ VirtualStorageId
		+ StackCount);
}

int32 UInventoryUtils::GetItemPrivateId(const FString& NewPayload,
                                        const TArray<int32>& NewPrivateIds,
                                        const int32 PhysicalGlobalId)
{
	NSJsonInventory::FJsonInventoryProvider OutProvider;
	NSJsonInventory::FromString(NewPayload, OutProvider);

	TArray<int32> FilteredSet = OutProvider.PrivateItemIds;
	for (const int32 PrivateId : NewPrivateIds)
	{
		FilteredSet.Remove(PrivateId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = PhysicalGlobalId](const int32 NewPrivateItemId)
	{
		// @gdemers filter the PrivateItemId that represent our complex encoding, and translate the virtual id parsed
		// from the integer into a physical id for comparison.
		const int32 OutPhysicalGlobalId = UItemObjectUtils::FilterItemPrivateId(NewPrivateItemId);
		return (false == (OutPhysicalGlobalId ^ SearchId))/*if both bits are identical, return 0.*/;
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

int32 UInventoryUtils::GetItemPrivateIdUsingStoragePosition(const FString& NewPayload,
                                                            const TArray<int32>& NewPrivateIds,
                                                            const int32 ItemStoragePosition)
{
	NSJsonInventory::FJsonInventoryProvider OutProvider;
	NSJsonInventory::FromString(NewPayload, OutProvider);

	TArray<int32> FilteredSet = OutProvider.PrivateItemIds;
	for (const int32 PrivateId : NewPrivateIds)
	{
		FilteredSet.Remove(PrivateId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchStorage = ItemStoragePosition](const int32 Value)
	{
		const int32 OutValue = UItemObjectUtils::FilterStoragePosition(Value);
		return (false == (OutValue ^ SearchStorage))/*if both bits are identical, return 0.*/;
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

int32 UInventoryUtils::GetObjectUniqueIdentifier(const UItemObject* Item)
{
	if (!IsValid(Item))
	{
		return INDEX_NONE;
	}
	else
	{
		const FDataRegistryId RegistryId = {UAVVMGameplaySettings::GetActorIdentifierRegistryType(), Item->BP_GetItemActorId().ItemName};
		return UAVVMGameplayUtils::GetActorUniqueIdentifierByRegistryId(RegistryId);
	}
}

int32 UInventoryUtils::TranslatePhysicalAddressing(const int32 RelationshipBitMask,
                                                   const int32 PhysicalGlobalId)
{
	constexpr int32 BitRange = GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE;
	constexpr int32 BitShift = GET_ELEMENT_VIRTUAL_GLOBAL_ID_RSHIFT;
	int32 BaseId = 0;

	if ((RelationshipBitMask & (1 << 0/*attachment bit-index*/)))
	{
		BaseId = (PhysicalGlobalId & ~GET_ATTACHMENT_PHYSICAL_ADDRESSING_OFFSET);
	}
	else if ((RelationshipBitMask & (1 << 2/*item bit-index*/)))
	{
		BaseId = (PhysicalGlobalId & ~GET_ITEM_PHYSICAL_ADDRESSING_OFFSET);
	}
	else if (false == !!RelationshipBitMask/*storage, or 000 bitmask*/)
	{
		BaseId = (PhysicalGlobalId & ~GET_STORAGE_PHYSICAL_ADDRESSING_OFFSET);
	}

	const int32 VirtualGlobalId = UAVVMOnlineEncodingUtils::EncodeInt32(BaseId, BitRange, BitShift);
	return VirtualGlobalId;
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
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IInventoryProvider::GetItemSrcType is None. Check if it was properly overriden.")))
	{
		return false;
	}

	return true;
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
