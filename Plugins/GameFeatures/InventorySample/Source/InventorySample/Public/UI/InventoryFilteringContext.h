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

#include "InventoryFilteringContext.generated.h"

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
	static TArray<UObject*> GetArrayByFilterContext(const FGameplayTagContainer& NewFilteringRules,
	                                                const TArray<UObject*>& NewObjects);
};
