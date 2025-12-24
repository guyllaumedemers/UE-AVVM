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

#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMOnlineEncoding.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMOnlineEncodingUtils is a utility blueprint library that expose reusable api for encoding integers with extra data.
 */
UCLASS()
class AVVMONLINE_API UAVVMOnlineEncodingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	int32 DecodeInt32(const int32 Input, const int32 BitRange, const int32 RShift);
};

// Bits encoding breakdown
//
// How do I want to encode my items.
//
// I want it to fit within an 'int32', thats 4Bytes- 32Bits.
//
// 2^0 = 1
// 2^1 = 2
// 2^2 = 4
// 2^3 = 8
// 2^4 = 16
// 2^5 = 32
// 2^6 = 64
// 2^7 = 128
// 2^8 = 256
// 2^9 = 512
// 2^10 = 1024
// 2^11 = 2048
// 2^12 = 4096
//
// Now the question  is -> How many items do I expect my game to hold ?
// the next question is -> What do I consider an Item ? (Are abilities, Perks, Skill, etc... Items ?)
// 
// Keep in mind that Items are referencing an AttributeSet which allow built-in progression. Limiting the number of unique items may not be
// a bad idea.
//
// lets attempt scheming using 2^12. Which gives designers 4096 unique entries for Items.
//
// if we want items to have abilities : exam - fire sword, the fire ability should be considered a mod to the item. with that in mind,
// abilities would be considered Mods.

// so first 12 bits are reserved to unique item id. (this DO NOT includes storage.)
// 
// A storage has an id, and is an item BUT it needs to be encoded separatly from the item id range.
//
// storage: 001 (id) << 12; lets say a storage id range is -> 2^3 = 8 unique id, reserving 3 bits.
//
// what will be our storage size ? (storage size is fixed but encoding may decide to not use all 6 bits)
//
// 2^6 = 64 bits. 6 bits offset.
//
// current total : 12 + 3 + 6 = 21 bits uses from our 32 reserved.
//
// how do we define the position within the storage.
//
// if size = 6 bits reserve. position is identical.
//
// 21 + 6 = 27 bits
//
// left 5 bits.
//
// item count = 5 remaining bits which equal 2^5 = 32 max stack count. we can clamp to 30.
