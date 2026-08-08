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

#include "InventoryStubDataProviderTableRow.generated.h"

class UItemObject;

/**
 *	Class description:
 *	
 *	FComplexDependencies are user defined values that participate in the generation of a complex
 *	bitmask scheme representing a dependency between an attachment, and an item.  (See AVVMOnlineInventory.h)
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FComplexDependencies
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<TSoftClassPtr<UItemObject>> Dependencies{};
};

/**
 *	Class description:
 *	
 *	FComplexDependencyLookupStubDataTableRow is a Row type that define the dependency between a UItemObject class,
 *	and many attachments.
 *	
 *	IMPORTANT : THIS ROW TYPE IS PURELY FOR BACKEND SIMULATION DURING HEADLESS PROJECT DEVELOPMENT! IF YOU REQUIRE
 *	DEFINING A STATIC DEPENDENCY BETWEEN AN ITEM AND AN ATTACHMENT, SIMPLY DEFINE THE DEPENDENCY IN YOUR ITEM REFERENCED ACTOR CLASS.
 *	(See WeaponSample ATriggeringActor for reference.)
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FComplexDependencyLookupStubDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TMap<TSoftClassPtr<UItemObject>, FComplexDependencies> ComplexDependencyLookup{};
};
