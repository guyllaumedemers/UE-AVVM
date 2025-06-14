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

#include "Components/ActorComponent.h"

#include "ActorInventoryComponent.generated.h"

/**
 *	UActorInventoryComponent handle the core functionalities of the inventory system.
 */
UCLASS(ClassGroup=("Inventory"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSAMPLE_API UActorInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	// what should this thing be able to do ?

	// A) Retrieve a collection of items from a backend.

	// B) Retrieve a collection of items from a data asset

	// C) Display a collection of items for the local client. (and optionally for a remote client that request a trade)

	// D) Broadcast event when item state is marked as equip, unequip, sell, buy, refund

	// the inventory system can act both as a caching mechanism for persistent data that require display, but also temporary for
	// items that can be bought, sold, traded etc...
};
