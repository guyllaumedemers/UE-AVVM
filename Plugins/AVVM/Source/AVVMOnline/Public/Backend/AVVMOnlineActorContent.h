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

#include "AVVMOnlineActorContent.generated.h"

/**
 *	Class description:
 *
 *	FAVVMItemModifier is the POD representation of a modifier being applied to an item. This POD defines the data layout
 *  stored on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMItemModifier
{
	GENERATED_BODY()

	bool operator==(const FAVVMItemModifier& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMPlayerResource.UniqueId}.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 ResourceId = INDEX_NONE;
};

/**
 *	Class description:
 *
 *	FAVVMItem is the RUNTIME POD representation of the item that's bound to an Actor. This POD defines the data layout
 *  stored on the backend.
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

	// @gdemers {FItemModifier.UniqueId} the POD applied by the player profile to this item. may be null.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> ModIds;
};

/**
 *	Class description:
 *	
 *	UAVVMOnlineInventoryUtils is a utility blueprint library that expose reusable api.
 */
UCLASS()
class AVVMONLINE_API UAVVMOnlineInventoryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
