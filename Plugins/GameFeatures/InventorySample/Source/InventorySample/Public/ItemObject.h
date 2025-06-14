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
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *	
 *	EItemStorageState represent the location state of an item. Where is the item
 *	being kept.
 */
UENUM(BlueprintType)
enum class EItemStorageState : uint8
{
	None,
	InGameWorld,
	InUserBackpackStorage,
	InUserBackupStorage
};

inline const TCHAR* EnumToString(EItemStorageState State)
{
	switch (State)
	{
		case EItemStorageState::InGameWorld:
			return TEXT("InGameWorld");
		case EItemStorageState::InUserBackpackStorage:
			return TEXT("InUserBackpackStorage");
		case EItemStorageState::InUserBackupStorage:
			return TEXT("InUserBackupStorage");
	}
	ensure(false);
	return TEXT("Unknown");
}

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
	EItemStorageState ItemStorageState = EItemStorageState::None;

	UPROPERTY(Transient, BlueprintReadOnly)
	EItemState ItemState = EItemState::None;
};
