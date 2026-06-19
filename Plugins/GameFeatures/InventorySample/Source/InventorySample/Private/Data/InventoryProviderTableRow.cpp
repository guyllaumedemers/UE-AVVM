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
#include "Data/InventoryProviderTableRow.h"

#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_INVENTORYSAMPLE_ITEM_RELATIONSHIP_ATTACHMENT, TEXT("PrivateItemId.Relationship.Attachment"));
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_INVENTORYSAMPLE_ITEM_RELATIONSHIP_CHARACTER, TEXT("PrivateItemId.Relationship.Character"));
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_INVENTORYSAMPLE_ITEM_RELATIONSHIP_ITEM, TEXT("PrivateItemId.Relationship.Item"));

const int32 FProviderDefaultItemProperties::GetRelationshipBitmask() const
{
	int32 Bitmask = 0; // @gdemers 0 is storage by design
	if (RelationshipTags.HasAnyExact(FGameplayTagContainer(TAG_INVENTORYSAMPLE_ITEM_RELATIONSHIP_ATTACHMENT)))
	{
		Bitmask += (1/*2^0*/);
	}

	if (RelationshipTags.HasAnyExact(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_RELATIONSHIP_CHARACTER}))
	{
		Bitmask += (2/*2^1*/);
	}

	if (RelationshipTags.HasAnyExact(FGameplayTagContainer{TAG_INVENTORYSAMPLE_ITEM_RELATIONSHIP_ITEM}))
	{
		Bitmask += (4/*2^2*/);
	}

	return Bitmask;
}

#if WITH_EDITOR
EDataValidationResult FInventoryProviderTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (!InventoryProviderActorIdentifierId.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FInventoryProviderTableRow", "", "FDataRegistryId missing. No valid RegistryId specified!"));
	}

	if (bCanInventoryProviderEquipItems && DefaultSlotTags.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FInventoryProviderTableRow", "", "Slot Tags empty. No valid Slot Tags specified!"));
	}

	if (DefaultInventory.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FInventoryProviderTableRow", "", "DefaultInventory empty. No valid entries specified!"));
	}

	return Result;
}
#endif
