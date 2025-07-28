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

#include "ActorContent.generated.h"

/**
 *	Class description:
 *
 *	FItemHolder is a backend POD representation of the items held. 
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemHolder
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 MaxSlots = INDEX_NONE;

	// @gdemers the position used to visualize a holder (example : a bag, tabulation, etc...) in UI.
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 Position = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<int32> ItemIds;
};

/**
 *	Class description:
 *
 *	FActorContent is a backend POD representation of the content an Actor has ownership on.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FActorContent
{
	GENERATED_BODY()

	// @gdemers the unique identifier of the actor content. For cases like storage actors from which we buy/sell/trade with, the content
	// may be shared across instances in a level using a share unique id that can be used to reference the actor type.
	// Note : int32 can be converted to an enum if required by your project. Prob. what I would do for shared id.
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<int32> ItemHolderIds;
};
