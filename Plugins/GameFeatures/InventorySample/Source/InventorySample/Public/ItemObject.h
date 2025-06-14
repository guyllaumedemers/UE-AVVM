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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "ItemObject.generated.h"

/**
 *	Class description:
 *	
 *	EItemState represent the state of an item.
 */
UENUM(BlueprintType)
enum class EItemState : uint8
{
	None,
	Equipped,
	PendingForTrade,
	Traded,
	PendingForSell,
	Sold,
	PendingForPurchase,
	Purchased,
	PendingForDiscard,
	Discarded,
	PendingForConsumption,
	Consumed
};

inline const TCHAR* EnumToString(EItemState State)
{
	switch (State)
	{
		case EItemState::Equipped:
			return TEXT("Equipped");
		case EItemState::PendingForTrade:
			return TEXT("PendingForTrade");
		case EItemState::Traded:
			return TEXT("Traded");
		case EItemState::PendingForSell:
			return TEXT("PendingForSell");
		case EItemState::Sold:
			return TEXT("Sold");
		case EItemState::PendingForPurchase:
			return TEXT("PendingForPurchase");
		case EItemState::Purchased:
			return TEXT("Purchased");
		case EItemState::PendingForDiscard:
			return TEXT("PendingForDiscard");
		case EItemState::Discarded:
			return TEXT("Discarded");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *	
 *	EItemDropBehaviourType represent the behaviour state of an item. What does the item execute when dropped.
 */
UENUM(BlueprintType)
enum class EItemDropBehaviourType : uint8
{
	None,
	DropAndPersist,
	DropAndDestroy
};

inline const TCHAR* EnumToString(EItemDropBehaviourType Type)
{
	switch (Type)
	{
		case EItemDropBehaviourType::DropAndPersist:
			return TEXT("DropAndPersist");
		case EItemDropBehaviourType::DropAndDestroy:
			return TEXT("DropAndDestroy");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *	
 *	EItemConsumptionState represent the state of an item. Can we consume the object.
 */
UENUM(BlueprintType)
enum class EItemConsumptionState : uint8
{
	None,
	CanBeConsumed
};

inline const TCHAR* EnumToString(EItemConsumptionState State)
{
	switch (State)
	{
		case EItemConsumptionState::CanBeConsumed:
			return TEXT("CanBeConsumed");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *	
 *	EItemStorageType represent the location type of an item. Where is the item being kept.
 */
UENUM(BlueprintType)
enum class EItemStorageType : uint8
{
	None,
	InGameWorld,
	InUserBackpackStorage,
	InUserBackupStorage
};

inline const TCHAR* EnumToString(EItemStorageType Type)
{
	switch (Type)
	{
		case EItemStorageType::InGameWorld:
			return TEXT("InGameWorld");
		case EItemStorageType::InUserBackpackStorage:
			return TEXT("InUserBackpackStorage");
		case EItemStorageType::InUserBackupStorage:
			return TEXT("InUserBackupStorage");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	FInventoryItemStatusHandler describe the status of an Item.
 *
 *	TODO @gdemers I may update this to be working with tags so to support blocking certain tags
 *	for given scenario but idk yet.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FInventoryItemStatusHandler
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	EItemDropBehaviourType ItemDropBehaviourType = EItemDropBehaviourType::None;

	UPROPERTY(Transient, BlueprintReadOnly)
	EItemConsumptionState ItemConsumptionState = EItemConsumptionState::None;

	UPROPERTY(Transient, BlueprintReadOnly)
	EItemStorageType ItemStorageType = EItemStorageType::None;

	UPROPERTY(Transient, BlueprintReadOnly)
	EItemState ItemState = EItemState::None;
};

/**
 *	Class description:
 *	
 *	UItemObject represent any object that exist in your project LORE. Note: When a UItemObject state
 *	becomes InWorld, an AActor is instanced on the server for player pick up.
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UItemObject : public UObject
{
	GENERATED_BODY()

public:
	virtual void SpawnItemActor(const TFunctionRef<AActor*(void)> Callback);

protected:
	// ItemActorClass represent any world object that is open for pick up. Note : We expect the actor to be receiving a UActorInteractionComponent to support both Interaction Actions,
	// and UAVVMResourceManagerComponent for Resource loading, via a GameFeaturePlugin Data Asset.
	// Note : Native Actor Class isn't required here as any BP Class can act as the receiver to the components being dynamically added.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> ItemActorClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<AActor> ItemActor = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FInventoryItemStatusHandler ItemStatusHandler;
};
