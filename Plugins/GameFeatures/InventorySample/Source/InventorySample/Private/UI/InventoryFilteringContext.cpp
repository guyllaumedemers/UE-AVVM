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
#include "UI/InventoryFilteringContext.h"

#include "InventorySettings.h"
#include "ItemObject.h"

TArray<UObject*> UInventoryConversionFunction::GetStorageItems(const TArray<UObject*>& NewObjects)
{
	return UInventoryConversionFunction::GetArrayByFilterContext(UInventorySettings::GetStorageRuleset(),
	                                                             NewObjects);
}

TArray<UObject*> UInventoryConversionFunction::GetEquippedItems(const TArray<UObject*>& NewObjects)
{
	return UInventoryConversionFunction::GetArrayByFilterContext(UInventorySettings::GetEquippedRuleset(),
	                                                             NewObjects);
}

TArray<UObject*> UInventoryConversionFunction::GetPassiveItems(const TArray<UObject*>& NewObjects)
{
	return UInventoryConversionFunction::GetArrayByFilterContext(UInventorySettings::GetPassiveRuleset(),
	                                                             NewObjects);
}

TArray<UObject*> UInventoryConversionFunction::GetOffensiveItems(const TArray<UObject*>& NewObjects)
{
	return UInventoryConversionFunction::GetArrayByFilterContext(UInventorySettings::GetOffensiveRuleset(),
	                                                             NewObjects);
}

TArray<UObject*> UInventoryConversionFunction::GetDefensiveItems(const TArray<UObject*>& NewObjects)
{
	return UInventoryConversionFunction::GetArrayByFilterContext(UInventorySettings::GetDefensiveRuleset(),
	                                                             NewObjects);
}

TArray<UObject*> UInventoryConversionFunction::GetConsumables(const TArray<UObject*>& NewObjects)
{
	return UInventoryConversionFunction::GetArrayByFilterContext(UInventorySettings::GetConsumableRuleset(),
	                                                             NewObjects);
}

TArray<UObject*> UInventoryConversionFunction::GetArrayByFilterContext(const FGameplayTagContainer& NewFilteringRules,
                                                                       const TArray<UObject*>& NewObjects)
{
	TArray<UObject*> OutResult;
	for (auto Iterator = NewObjects.CreateConstIterator(); Iterator; ++Iterator)
	{
		auto* NewItem = Cast<UItemObject>(*Iterator);
		if (IsValid(NewItem) && NewItem->DoesTypeHasPartialMatch(NewFilteringRules))
		{
			OutResult.Add(*Iterator);
		}
	}

	return OutResult;
}
