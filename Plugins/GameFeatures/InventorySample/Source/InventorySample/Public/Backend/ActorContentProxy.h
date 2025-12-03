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

#include "ActorContentProxy.generated.h"

/**
 *	Class description:
 *
 *	FItemModifierProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemModifierProxy
{
	GENERATED_BODY()

	bool operator==(const FItemModifierProxy& Rhs) const;

	// @gdemers UniqueId here is used to retrieve the entry on the backend to support data reordering in parent array.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistryId}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ResourceId = FString();
};

/**
 *	Class description:
 *
 *	FItemProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemProxy
{
	GENERATED_BODY()

	bool operator==(const FItemProxy& Rhs) const;

	// @gdemers UniqueId here is used to retrieve the entry on the backend to support data reordering in parent array.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FDataRegistryId}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ResourceId = FString();

	// @gdemers {FItemModifierProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> ModValues;
};

/**
 *	Class description:
 *
 *	FItemHolderProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemHolderProxy
{
	GENERATED_BODY()

	bool operator==(const FItemHolderProxy& Rhs) const;

	// @gdemers UniqueId here is used to retrieve the entry on the backend to support data reordering in parent array.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FItemProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> ItemValues;
};

/**
 *	Class description:
 *
 *	FActorContentProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FActorContentProxy
{
	GENERATED_BODY()

	bool operator==(const FActorContentProxy& Rhs) const;

	// @gdemers UniqueId here is used to retrieve the entry on the backend and apply ordering changes to children based on local array ordering.
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FItemHolderProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> ItemHolderValues;
};
