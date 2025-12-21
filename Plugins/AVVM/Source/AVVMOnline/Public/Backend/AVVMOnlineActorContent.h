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
 *	FAVVMItemHolder is a backend POD representation of the items held. You can look at it like a bag and it's content,
 *	or a tabulation and it's items, or a NPC equipped items, etc... This POD defines the data layout stored on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMItemHolder
{
	GENERATED_BODY()

	bool operator==(const FAVVMItemHolder& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// TODO @gdemers may want to revert latest change to allow bags (i.e ItemHolder) to reference their
	// resource definition. i.e How many items can they hold, etc...

	// @gdemers {FItem.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> ItemIds;
};

/**
 *	Class description:
 *
 *	FAVVMActorContent is a backend POD representation of the content an Actor has ownership on. This POD defines the data layout
 *	stored on the backend.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMActorContent
{
	GENERATED_BODY()

	bool operator==(const FAVVMActorContent& Rhs) const;

	// @gdemers the unique identifier of the actor content. For cases like storage actors from which we buy/sell/trade with, the content
	// may be shared across instances in a level using a share unique id that can be used to reference the actor type.
	// Note : int32 can be converted to an enum if required by your project. Prob. what I would do for shared id.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FItemHolder.UniqueId}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<int32> ItemHolderIds;
};
