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
#include "AutomatedTestInventoryActor.h"

#include "ActorInventoryComponent.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "AutomatedTest/AVVMAutomatedTestResourceValidationManager.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Engine/StreamableManager.h"
#include "Resources/InventoryResourceHandlingImpl.h"

AAutomatedTestInventoryActor::AAutomatedTestInventoryActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));
	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UActorInventoryComponent>(this, TEXT("InventoryComponent"));
}

int32 AAutomatedTestInventoryActor::GetProviderUniqueId_Implementation() const
{
	// @gdemers Hard coded in our ActorIdentifier Data Table.
	return 0;
}

UAVVMResourceManagerComponent* AAutomatedTestInventoryActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> AAutomatedTestInventoryActor::GetResourceDefinitionRegistryIds_Implementation() const
{
	const auto TestRegistryId = FDataRegistryId(UInventorySettings::GetItemGroupRegistryType(), TEXT("DEMO_InventorySample_AutomatedTest_C"));
	ensureAlwaysMsgf(TestRegistryId.IsValid(), TEXT("RegistryType or ItemName arent valid. Please validate your Data Registry."));
	return {TestRegistryId};
}

TArray<FDataRegistryId> AAutomatedTestInventoryActor::CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const
{
	TArray<FDataRegistryId> OutResults;
	OutResults.Append(UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(
	                                                                                UInventoryResourceHandlingImpl::StaticClass(),
	                                                                                InventoryComponent,
	                                                                                Resources));

	return OutResults;
}

EItemSrcType AAutomatedTestInventoryActor::GetItemSrcType_Implementation() const
{
	return EItemSrcType::Static;
}

bool AAutomatedTestInventoryActor::DoesSupportSpawnOnLoad_Implementation() const
{
	return false;
}

void AAutomatedTestInventoryActor::SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted)
{
	bIsAsyncProcessCompleted = bNewIsAsyncProcessCompleted;
}

bool AAutomatedTestInventoryActor::CheckContentIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), ResourceManagerComponent);
}

bool AAutomatedTestInventoryActor::CheckInventoryIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), InventoryComponent);
}

bool AAutomatedTestInventoryActor::HasInventoryFinishedAllStreaming() const
{
	if (!IsValid(InventoryComponent))
	{
		return true;
	}

	bool bHasFinishedStreaming = true;
	for (const auto& [TokenId, StreamingHandle] : InventoryComponent->ItemHandleSystem)
	{
		bHasFinishedStreaming &= (StreamingHandle.IsValid() && StreamingHandle->HasLoadCompleted());
	}

	return bHasFinishedStreaming && (InventoryComponent->Items.Num() == InventoryComponent->ItemHandleSystem.Num());
}

void AAutomatedTestInventoryActor::ForceCompletion() const
{
	(*bIsAsyncProcessCompleted) = true;
}

FOnResourceAsyncLoadingComplete AAutomatedTestInventoryActor::GetOnCompleteDelegate()
{
	FOnResourceAsyncLoadingComplete Callback;
	Callback.BindDynamic(this, &AAutomatedTestInventoryActor::OnRequestCompleted);
	return Callback;
}

bool AAutomatedTestInventoryActor::RunTest_ItemUniqueId(const TArray<const FAVVMActorIdentifierDataTableRow*>& ActorIdentifiers) const
{
	if (!IsValid(InventoryComponent) || InventoryComponent->GetItems().IsEmpty())
	{
		return false;
	}

	const auto CheckActorIdentifier = [](const int32 ActorIdentifier, const TArray<const FAVVMActorIdentifierDataTableRow*>& Rows)
	{
		const auto* SearchResult = Rows.FindByPredicate([ActorIdentifier](const FAVVMActorIdentifierDataTableRow* Row)
		{
			return (Row != nullptr) && (Row->UniqueId == ActorIdentifier);
		});

		return (SearchResult != nullptr);
	};

	bool bResult = true;
	for (const UItemObject* ItemObject : InventoryComponent->GetItems())
	{
		if (!IsValid(ItemObject))
		{
			continue;
		}

		const int32 NonShiftedItemId = UItemObjectUtils::FilterItem(ItemObject);
		bResult &= CheckActorIdentifier(NonShiftedItemId, ActorIdentifiers);
	}

	return bResult;
}

bool AAutomatedTestInventoryActor::RunTest_ItemStorageReference() const
{
	if (!IsValid(InventoryComponent) || InventoryComponent->GetItems().IsEmpty())
	{
		return false;
	}

	bool bResult = true;
	for (const UItemObject* ItemObject : InventoryComponent->GetItems())
	{
		if (!IsValid(ItemObject))
		{
			continue;
		}

		const int32 PrivateItemId = UItemObjectUtils::GetPrivateItemId(ItemObject);
		if (!UItemObjectUtils::IsStorage(PrivateItemId))
		{
			continue;
		}

		const int32 ShiftedStorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
		const int32 MaxStorageCapacity = UItemObjectUtils::GetStorageMaxCapacity(InventoryComponent.Get(), ShiftedStorageId);
		
		// @gdemers test that the max storage capacity retrieved for this storage item is indeed handling correctly being maxed out.
		bResult &= (UItemObjectUtils::HasStorageReachMaxCapacity(InventoryComponent.Get(), ShiftedStorageId, MaxStorageCapacity));
		bResult &= (MaxStorageCapacity > 0);
	}
	
	// @gdemers create test object (is unreferenced when going out of scope implying gc will eventually release memory)
	auto* TestObject = UItemObjectUtils::MakeZeroInitItemObject();
	
	{
		// @gdemers test setting storage on an item in world.
		FStorageQualifierContextArgs Params;
		Params.PrivateItemIds = InventoryComponent->PrivateItemIds;
		Params.StoragePositionBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_POSITION_ENCODING_BIT_RANGE);
		Params.StorageIdBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_ID_ENCODING_BIT_RANGE);
		Params.CurrentStoragePosition = INDEX_NONE;
		Params.CurrentStorageId = INDEX_NONE;

		// @gdemers encode new storage position into UItemObject based on inventory available layout.
		UItemObjectUtils::QualifyStorage(InventoryComponent.Get(), Params, TestObject);

		const int32 PrivateItemId = UItemObjectUtils::GetPrivateItemId(TestObject);
		const int32 ShiftedStorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
		const int32 ShiftedStoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE, GET_ITEM_POSITION_ENCODING_RSHIFT);

		// @gdemers test assigned storage id.
		const int32 StorageIdBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_STORAGE_ID_ENCODING_BIT_RANGE);
		bResult &= ((ShiftedStorageId < StorageIdBounds) && (ShiftedStorageId > 0));

		// @gdemers test assigned storage position.
		const int32 StoragePositionBounds = UItemObjectUtils::GetStorageMaxCapacity(InventoryComponent.Get(), ShiftedStorageId);
		bResult &= (ShiftedStoragePosition < StoragePositionBounds) && (ShiftedStoragePosition > 0);

		// @gdemers validate if theres two items with the same positions. Note : Our test item
		// isnt part of the existing Item collection, each position entry should be unique.
		for (const UItemObject* ItemObject : InventoryComponent->GetItems())
		{
			const int32 NewPrivateItemId = UItemObjectUtils::GetPrivateItemId(ItemObject);
			if (UItemObjectUtils::IsStorage(NewPrivateItemId))
			{
				continue;
			}

			const int32 NewShiftedStoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(NewPrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE, GET_ITEM_POSITION_ENCODING_RSHIFT);
			if (false == (NewShiftedStoragePosition ^ ShiftedStoragePosition))
			{
				bResult &= false;
			}
		}
	}
	
	{
		// @gdemers test nulifying storage on an item.
		UItemObjectUtils::NullifyStorage(TestObject);

		const int32 PrivateItemId = UItemObjectUtils::GetPrivateItemId(TestObject);
		const int32 ShiftedStorageId = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_STORAGE_ID_ENCODING_BIT_RANGE, GET_STORAGE_ID_ENCODING_RSHIFT);
		const int32 ShiftedStoragePosition = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_POSITION_ENCODING_BIT_RANGE, GET_ITEM_POSITION_ENCODING_RSHIFT);

		bResult &= (ShiftedStorageId == 0);
		bResult &= (ShiftedStoragePosition == 0);
	}

	return bResult;
}

bool AAutomatedTestInventoryActor::RunTest_ItemStacking() const
{
	if (!IsValid(InventoryComponent) || InventoryComponent->GetItems().IsEmpty())
	{
		return false;
	}

	bool bResult = true;
	for (const UItemObject* ItemObject : InventoryComponent->GetItems())
	{
		if (!IsValid(ItemObject))
		{
			continue;
		}

		const int32 PrivateItemId = UItemObjectUtils::GetPrivateItemId(ItemObject);
		const int32 ItemStackCount = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_COUNT_ENCODING_BIT_RANGE, GET_ITEM_COUNT_ENCODING_RSHIFT);
		bResult &= (ItemStackCount > 0);
	}

	return bResult;
}

void AAutomatedTestInventoryActor::OnRequestCompleted()
{
	// @gdemers Our async process is complete. All resources were loaded!
	ForceCompletion();
}
