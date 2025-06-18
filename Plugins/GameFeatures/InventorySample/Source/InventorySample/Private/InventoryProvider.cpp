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
#include "InventoryProvider.h"

#include "AVVMUtilityFunctionLibrary.h"

void UInventoryBlueprintFunctionLibrary::RequestItems(const UObject* Outer,
                                                      const FOnRetrieveInventoryItems& Callback)
{
	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IInventoryProvider, UInventoryProvider>(Outer);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IInventoryProvider interface!")))
	{
		return;
	}

	const EItemSrcType ItemSrcType = IInventoryProvider::Execute_GetItemSrcType(Outer);
	const bool bIsNone = EnumHasAnyFlags(ItemSrcType, EItemSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IHasItemCollection::GetItemSrcType is None. Check if it was properly overriden.")))
	{
		Callback.ExecuteIfBound({});
		return;
	}

	const bool bIsItemSrcStatic = EnumHasAnyFlags(ItemSrcType, EItemSrcType::Static);
	if (bIsItemSrcStatic)
	{
		// @gdemers manage items retrieved from data asset
		IInventoryProvider::Execute_ProcessStaticItems(Outer, Callback);
	}
	else
	{
		// @gdemers manage items retrieved from backend services
		IInventoryProvider::Execute_ProcessDynamicItems(Outer, Callback);
	}
}

void UInventoryBlueprintFunctionLibrary::ExecuteInventoryProviderDelegate(const TArray<UItemObject*>& NewItems,
                                                                          const FOnRetrieveInventoryItems& Callback)
{
	Callback.ExecuteIfBound(NewItems);
}
