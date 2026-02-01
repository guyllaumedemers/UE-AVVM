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

#include "InventoryUtils.generated.h"

struct FDataRegistryId;
struct FStorageHelper;
class UItemObject;

/**
 *	Class description:
 *
 *	UInventoryUtils expose a set of utility function relevant to the inventory system, and the items
 *	backend representation.
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static bool GetOuterSourceType(const AActor* Outer, EItemSrcType& OutSrcType);
	
	UFUNCTION(BlueprintCallable)
	static int32 DecodeItem(const int32 EncodedBits);
	
	UFUNCTION(BlueprintCallable)
	static TArray<int32> GetRuntimeUniqueIds(const TArray<UItemObject*>& Items);
	
	// @gdemers IMPORTANT : the Unique Identifier is not a shifted value. the inventory system handle
	// shifting following user defined rules.
	UFUNCTION(BlueprintCallable)
	static int32 GetObjectUniqueIdentifier(const UItemObject* Item);
	
	UFUNCTION(BlueprintCallable)
	static int32 GetItemActorUniqueIdentifier(const FDataRegistryId& ItemActorId);

	UFUNCTION(BlueprintCallable)
	static FString ModifyProfile(const UObject* WorldContextObject,
	                             const int32 ProfileId,
	                             const TArray<int32>& NewItems);

	UFUNCTION(BlueprintCallable)
	static TArray<FString> GetInventoryProviderPayloads(const FString& NewPayload/*FileContent*/);

	UFUNCTION(BlueprintCallable)
	static FString GetInventoryProviderById(const FString& NewPayload/*FileContent*/,
	                                        const int32 NewProviderId);

	UFUNCTION(BlueprintCallable)
	static int32 GetItemPrivateId(const FString& NewPayload,
	                              const TArray<int32>& NewPrivateIds,
	                              const int32 ItemId);

	UFUNCTION(BlueprintCallable)
	static FString ModifyInventoryProvider(const FString& NewPayload/*FileContent*/,
	                                       const int32 ProviderId,
	                                       const TArray<int32>& NewPrivateIds);

	UFUNCTION(BlueprintCallable)
	static FString CreateDefaultInventoryProviders();

	UFUNCTION(BlueprintCallable)
	static int32 CreateDefaultPrivateItemId(const UItemObject* ItemObjectCDO,
	                                        const int32 StackCount);
};
