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

#include "AVVMActorIdentifierTableRow.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMActorIdentifierDataTableRow is a table row entry thet define a unique identifier for a given Actor type. Doing so allows
 *	our Backend encoding system to reference elements, and create complex encoding scheme for tightly packing data.
 *	
 *	Note : We do expect some rules to be followed. Make sure the RowName define for this entry match the Actor Class FName, and if you are
 *	using the InventorySample system, make sure the FDataRegistryId::ItemName is identical to the Actor Class FName referenced.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorIdentifierDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// TODO @gdemers make editor tooling that allow default assignment of next value in sequence, and can target Attachment bits, Storage bits, and items.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;
};