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

void IInventoryProvider::RequestItems_Implementation(const FOnRetrieveInventoryItems& Callback) const
{
	const EItemSrcType ItemSrcType = GetItemSrcType();
	const bool bIsNone = EnumHasAnyFlags(ItemSrcType, EItemSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IHasItemCollection::GetItemSrcType is None. Check if it was properly overriden.")))
	{
		Callback.ExecuteIfBound({});
		return;
	}

	const bool bIsItemSrcStatic = EnumHasAnyFlags(ItemSrcType, EItemSrcType::Static);
	if (bIsItemSrcStatic)
	{
		// @gdemers expect loading of local resource based on FDataRegistryIds fetch from static data. Note : UAVVMResourceManagerComponent
		// should be invoked and handle resource loading at the Actor level in the override.
		// Note : IResourceProvider return a RegistryId defined by the owning Actor. This can be used for static data that require loading
		// during the ResourceComponent OnBeginPlay.
		// TODO @gdemers Resource loading on an Actor with Static data may be already running on the Resource Component due to IResourceProvider defining the RegistryId.
		// the Component should be able to store callback delegates, wait until completion of all resources loading and broadcast all registered callbacks.
		ProcessStaticItems(Callback);
	}
	else
	{
		// @gdemers expect handling of any backend request and loading of resources based on FDataRegistryIds received post-request completion. Note : UAVVMResourceManagerComponent
		// should be invoked and handle resource loading at the Actor level in the override.
		ProcessDynamicItems(Callback);
	}
}
