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

#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#include "InventoryFilteringContext.generated.h"

/**
 *	Class description:
 *
 *	FInventoryFilteringContext is the base filtering context that abstract the implementation details for using conversion function with View model bindings.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual ~FInventoryFilteringContext() = default;

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const PURE_VIRTUAL(GetReduceSet, return TArray<UObject*>{};);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FAVVMNotificationPayload>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FInventoryFilteringContext> Make(TArgs&&... Args);
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FInventoryFilteringContext> FInventoryFilteringContext::Make(TArgs&&... Args)
{
	return TInstancedStruct<FInventoryFilteringContext>::Make<TChild>(Forward<TArgs>(Args)...);
}

/**
 *	Class description:
 *
 *	FEquipFilteringContext is a filtering context object that parses a collection set and returns all items that are
 *	stored.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FStorageFilteringContext : public FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const;
};

/**
 *	Class description:
 *
 *	FEquipFilteringContext is a filtering context object that parses a collection set and returns all items that are
 *	equipped.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FEquipFilteringContext : public FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const;
};

/**
 *	Class description:
 *
 *	FPassiveFilteringContext is a filtering context object that parses a collection set and returns all items that are
 *	passive.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FPassiveFilteringContext : public FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const;
};

/**
 *	Class description:
 *
 *	FOffensiveFilteringContext is a filtering context object that parses a collection set and returns all items that are
 *	offensive.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FOffensiveFilteringContext : public FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const;
};

/**
 *	Class description:
 *
 *	FDefensiveFilteringContext is a filtering context object that parses a collection set and returns all items that are
 *	defensive.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FDefensiveFilteringContext : public FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const;
};

/**
 *	Class description:
 *
 *	FConsumableFilteringContext is a filtering context object that parses a collection set and returns all items that are
 *	consumable.
 */
USTRUCT()
struct INVENTORYSAMPLE_API FConsumableFilteringContext : public FInventoryFilteringContext
{
	GENERATED_BODY()

	virtual TArray<UObject*> GetReduceSet(const TArray<UObject*>& NewObjects,
	                                      const FGameplayTagContainer& NewRequirements) const;
};

/**
 *	Class description:
 *
 *	UAVVMConversionFunction are View model bindings functions that allow filtering complex sets
 *	into reduced form.
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryConversionFunction : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<UObject*> GetStorageItems(const TArray<UObject*>& NewObjects);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<UObject*> GetEquippedItems(const TArray<UObject*>& NewObjects);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<UObject*> GetPassiveItems(const TArray<UObject*>& NewObjects);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<UObject*> GetOffensiveItems(const TArray<UObject*>& NewObjects);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<UObject*> GetDefensiveItems(const TArray<UObject*>& NewObjects);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<UObject*> GetConsumables(const TArray<UObject*>& NewObjects);

private:
	static TArray<UObject*> GetArrayByFilterContext(const TInstancedStruct<FInventoryFilteringContext>& NewFilteringContext,
	                                                const FGameplayTagContainer& NewFilteringRules,
	                                                const TArray<UObject*>& NewObjects);
};
