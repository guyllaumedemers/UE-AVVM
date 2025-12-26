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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "AVVMOnlineInventory.generated.h"

// @gdemers passive items that are direct child of character require their bits encoding to set the first 12 bits to the max value
// so to allow validation during the attachment process.
#ifndef CHECK_CHARACTER_DEPENDENT_ENCODING
#define EXPECTED_VALUE (1 << 12)
#define CHECK_CHARACTER_DEPENDENT_ENCODING EXPECTED_VALUE
#endif

/**
 *	Class description:
 *
 *	FAVVMItem is the POD representation of the item that's bound to an Actor. This POD defines the data layout
 *  stored on the backend.
 *  
 *  Note : Items, and mods are the same conceptually. {FAVVMPlayerProfile::InventoryIds} handle item composition
 *  using bits encoding, and can reconstruct an FAVVMItem, it's Mods, and storage location by parsing the input integer.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMItem
{
	GENERATED_BODY()

	bool operator==(const FAVVMItem& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMPlayerResource.UniqueId}.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 ResourceId = INDEX_NONE;
};
