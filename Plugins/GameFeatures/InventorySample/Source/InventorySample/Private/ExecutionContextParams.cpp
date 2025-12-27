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
#include "ExecutionContextParams.h"

#include "ActorInventoryComponent.h"
#include "ItemObject.h"
#include "NonReplicatedLoadoutObject.h"

UScriptStruct* TBaseStructure<FExecutionContextParams>::Get()
{
	return FExecutionContextParams::StaticStruct();
}

FDropContextParams::FDropContextParams(UItemObject* NewItemObject)
	: ItemObject(NewItemObject)
{
}

void FDropContextParams::Execute(UNonReplicatedLoadoutObject* NonReplicatedLoadoutObject) const
{
	if (!IsValid(NonReplicatedLoadoutObject))
	{
		return;
	}

	auto* InventoryComponent = NonReplicatedLoadoutObject->GetTypedOuter<UActorInventoryComponent>();
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnDrop(ItemObject.Get());
	}
}

FPickupContextParams::FPickupContextParams(UItemObject* NewItemObject)
	: ItemObject(NewItemObject)
{
}

void FPickupContextParams::Execute(UNonReplicatedLoadoutObject* NonReplicatedLoadoutObject) const
{
	if (!IsValid(NonReplicatedLoadoutObject))
	{
		return;
	}

	auto* InventoryComponent = NonReplicatedLoadoutObject->GetTypedOuter<UActorInventoryComponent>();
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnPickup(ItemObject.Get());
	}
}

FSwapContextParams::FSwapContextParams(UItemObject* NewSrcItemObject,
                                       UItemObject* NewDestItemObject)
	: SrcItemObject(NewSrcItemObject),
	  DestItemObject(NewDestItemObject)
{
}

void FSwapContextParams::Execute(UNonReplicatedLoadoutObject* NonReplicatedLoadoutObject) const
{
	if (!IsValid(NonReplicatedLoadoutObject))
	{
		return;
	}

	auto* InventoryComponent = NonReplicatedLoadoutObject->GetTypedOuter<UActorInventoryComponent>();
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnSwap(SrcItemObject.Get(),
		                           DestItemObject.Get());
	}
}
