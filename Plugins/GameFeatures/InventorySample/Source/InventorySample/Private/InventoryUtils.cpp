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
#include "ItemObject.h"
#include "Backend/AVVMDataResolverHelper.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorIdentifierTableRow.h"

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
