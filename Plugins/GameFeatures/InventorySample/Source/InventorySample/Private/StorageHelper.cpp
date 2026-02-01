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
#include "StorageHelper.h"

#include "ItemObject.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"

void FStorageHelper::HandleStorageAssignment(const TMap<int32, TWeakObjectPtr<const UItemObject>>& ItemCDOs,
                                             TArray<int32>& Items)
{
	const TArray<int32> SearchResults = Items.FilterByPredicate([](const int32 Value)
	{
		const int32 StorageId = UAVVMOnlineEncodingUtils::FilterInt32(Value, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
		return (!!StorageId);
	});

	if (!ensureAlwaysMsgf(!SearchResults.IsEmpty(),
	                      TEXT("Inventory Provider definition is not referencing a valid Storage UItemObject. Please add one!")))
	{
		return;
	}

	int32 StorageStartPosition = NULL;
	for (const int32 StoragePrivateItemId : SearchResults)
	{
		if (StorageStartPosition >= Items.Num())
		{
			return;
		}

		const bool bDoesContains = ItemCDOs.Contains(StoragePrivateItemId);
		if (!bDoesContains)
		{
			continue;
		}

		const TWeakObjectPtr<const UItemObject>& StorageObject = ItemCDOs[StoragePrivateItemId];
		if (!StorageObject.IsValid())
		{
			continue;
		}

		const int32 StorageMaxCapacity = StorageObject->GetStorageMaxCapacity();
		if (!ensureAlwaysMsgf(StorageMaxCapacity != INDEX_NONE,
		                      TEXT("Invalid Storage Capacity set. Make sure you have configured the required Data Table for Storage Capacity")))
		{
			continue;
		}

		const int32 FilteredStorageId = UAVVMOnlineEncodingUtils::FilterInt32(StoragePrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
		for (int32 i = StorageStartPosition; (i < Items.Num()) && (i < (StorageStartPosition + StorageMaxCapacity)); ++i)
		{
			const int32 StoragePosition = ((StorageStartPosition + i) % StorageMaxCapacity);
			Items[i] += (FilteredStorageId + UAVVMOnlineEncodingUtils::EncodeInt32(StoragePosition, GET_ITEM_POSITION_ENCODING_BIT_RANGE, GET_ITEM_POSITION_ENCODING_RSHIFT));
		}

		StorageStartPosition += StorageMaxCapacity;
	}
}
