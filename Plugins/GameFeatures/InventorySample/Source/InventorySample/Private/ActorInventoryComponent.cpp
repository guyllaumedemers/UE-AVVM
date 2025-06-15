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
#include "ActorInventoryComponent.h"

#include "AVVMUtilityFunctionLibrary.h"
#include "InventoryProvider.h"
#include "ItemObject.h"

void UActorInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	UObject* Outer = GetOuter();
	if (!IsValid(Outer))
	{
		return;
	}

	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IInventoryProvider, UInventoryProvider>(Outer);
	if (ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IInventoryProvider interface!")))
	{
		FOnRetrieveInventoryItems Callback;
		Callback.BindDynamic(this, &UActorInventoryComponent::OnItemsRetrieved);
		IInventoryProvider::Execute_RequestItems(Outer, Callback);
	}

	OwningActor = Outer;
}

void UActorInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	InventoryItems.Reset();
	OwningActor.Reset();
}

void UActorInventoryComponent::OnItemsRetrieved(const TArray<UItemObject*>& ItemObjectIds)
{
	if (!ensureAlwaysMsgf(!ItemObjectIds.IsEmpty(), TEXT("UActorInventoryComponent::OnItemsRetrieved has received an Empty Collection!")))
	{
		return;
	}

	const AActor* Owner = OwningActor.Get();
	if (!ensureAlwaysMsgf(IsValid(Owner), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	InventoryItems.Reset(ItemObjectIds.Num());
	InventoryItems = ItemObjectIds;

	// @gdemers Player Inventory should only spawn actor for equipped items that are in the primary slot (visible), other actor types
	// should only be able to spawn a visual representation of the item bases on system requirements, example : hovering over item in grid
	// trigger event that require spawning a mesh of the object in the world.
	for (UItemObject* Item : InventoryItems)
	{
		Item->TrySpawnEquippedItem(Owner);
	}
}
