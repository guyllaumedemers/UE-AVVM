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

#include "Engine/DataTable.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "ItemIdentifierTableRow.generated.h"

/**
 *	Class description:
 *	
 *	FItemIdentifierDataTableRow is a table row entry that pair a UItemObject::ItemActorClass derived BP
 *	to a unique identifier.
 *	
 *	This mapping allow user to access the dependent objects referenced in backend representation
 *	of an {FItem.UniqueId}. Example : {FItem::Mods}. Doing so allow resolving attachment socket targeting during inventory initialization.
 *	
 *	Note : In order to work, we require access to the Outer {FActorContent::UniqueId}.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemIdentifierDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<AActor> ItemActorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;
};
