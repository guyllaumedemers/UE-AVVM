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

#include "ExecutionContextParams.h"
#include "ExecutionContextRule.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#include "NonReplicatedLoadoutObject.generated.h"

class UItemObject;

/**
 *	Class description:
 *
 *	UNonReplicatedLoadoutObject is a system UObject that define the behaviour of a user loadout, and manage
 *	items 'equip' state.
 *
 *	Note : AI may extend this for more advance combat feature, so in-combat behaviour can change
 *	weapons based on inventory defined on Pawn.
 *
 *	Important : Item swapping, or cycling expect simulated events to happen both on Server and Client
 *	as the state of this object isnt expected to be replicated.
 *
 *		Example : AbilityTask_MouseWheelEvent. (*not an actual class*)
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UNonReplicatedLoadoutObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void HandleItemCollectionChanged(const TArray<UItemObject*>& NewItemObjects,
	                                 const TArray<UItemObject*>& OldItemObjects);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="Define the order in which mouse wheel scolling cycles through items."))
	TArray<FGameplayTag> CyclingSlots;

	UPROPERTY(Transient, meta=(ToolTip="Define the items that are in the Equipped state, but not necessarily active."))
	TMap<FGameplayTag, TWeakObjectPtr<UItemObject>> Loadout;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ToolTip="Define the slot tag of the item in hand."))
	FGameplayTag ActiveItemSlotTag = FGameplayTag::EmptyTag;

	friend class ULoadoutUtils;
};

/**
 *	Class description:
 *	
 *	ULoadoutUtils is a blueprint function library of reusable function for interfacing with the inventory.
 */
UCLASS()
class INVENTORYSAMPLE_API ULoadoutUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void Drop(UNonReplicatedLoadoutObject* NonReplicatedLoadoutObject,
	                 UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable)
	static void Pickup(UNonReplicatedLoadoutObject* NonReplicatedLoadoutObject,
	                   UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable)
	static void Swap(UNonReplicatedLoadoutObject* NonReplicatedLoadoutObject,
	                 UItemObject* SrcItemObject,
	                 UItemObject* DestItemObject);
};
