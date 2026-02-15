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
#pragma once

#include "CoreMinimal.h"

#include "DataRegistryId.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "InventoryProviderTableRow.generated.h"

class UItemObject;

/**
 *	Class description:
 *	
 *	FInventoryProviderTableRow is a Row type that define the default values used to initialize an
 *	Inventory Provider entry on disk (based on Data Asset).
 *	
 *	Notes : Item defined in this Row Table are unique elements. Complex scheme, such as a weapon with attachments
 *	are pre-baked from the Item Actor definition a UItemObject reference. There shouldn't be any relationship between items here!
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FInventoryProviderTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// @gdemers the unique identifier that represent the inventory provider actor. example : a shop.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorIdentifierDataTableRow"))
	FDataRegistryId InventoryProviderActorIdentifierId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="0", ClampMax="999"))
	TMap<TSoftClassPtr<UItemObject>, int32/*DefaultStackCount*/> DefaultInventory;
};
