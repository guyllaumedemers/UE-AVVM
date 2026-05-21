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
#include "InventoryFileHelper.h"
#include "InventorySettings.h"
#include "InventoryUtils.h"
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
	auto* TestObject = UItemObjectUtils::MakeZeroInitItemObject(InventoryComponent.Get());
	
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
		
		TestObject->ModifyRuntimeStorageId(ShiftedStorageId);
		TestObject->ModifyRuntimeStoragePosition(ShiftedStoragePosition);

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
		// @gdemers test serializing to disk changes made to the inventory system.
		InventoryComponent->Items.Add(TestObject);
		InventoryComponent->CheckDisk();

		// @gdemers get-set file from disk caching all inventory providers representation.
		const FStringView FileContent = UInventoryFileHelper::Static_GetSetFileContent();

		const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(this);
		const FString OutInventoryProviders = UInventoryUtils::GetInventoryProviderById(FileContent.GetData(), TargetUniqueId);

		const int32 NonShiftedItemId = UItemObjectUtils::FilterItem(TestObject);
		const int32 PrivateItemId = UInventoryUtils::GetItemPrivateId(OutInventoryProviders, {}, NonShiftedItemId);
		bResult &= (PrivateItemId != INDEX_NONE);

		// @gdemers revert changes made to the inventory system for subsequent cases.
		InventoryComponent->Items.Remove(TestObject);
	}
	
	{
		// @gdemers test nullifying storage on an item.
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

	UItemObject* SingleSplit = nullptr;

	bool bResult = true;
	for (UItemObject* ItemObject : InventoryComponent->GetItems())
	{
		if (!IsValid(ItemObject) || !ItemObject->CanStack(ItemObject))
		{
			continue;
		}

		const int32 PrivateItemId = UItemObjectUtils::GetPrivateItemId(ItemObject);

		// @gdemers test referencing a valid stack count. i.e non-zero.
		const int32 ItemStackCount = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_ITEM_COUNT_ENCODING_BIT_RANGE, GET_ITEM_COUNT_ENCODING_RSHIFT);
		bResult &= (ItemStackCount > 0);

		// @gdemers test splitting an object with count overflow.
		const int32 StackBounds = ItemObject->GetMaxStackCount();
		ItemObject->ModifyRuntimeStackCount(FMath::CeilToInt(StackBounds * 1.5), false/*enforce not clamping*/);

		const int32 NumSplits = UItemObjectUtils::GetNumSplits(ItemObject);
		bResult &= (NumSplits > 0);

		SingleSplit = UItemObjectUtils::SplitObject(InventoryComponent.Get(), ItemObject);
		if (IsValid(SingleSplit))
		{
			bResult &= (SingleSplit->GetRuntimeCount() == StackBounds);
			bResult &= (ItemObject->GetRuntimeCount() == FMath::CeilToInt(SingleSplit->GetRuntimeCount() * 1.5) - StackBounds);
		}

		// @gdemers test the api for stacking.
		const bool bHasOverflow = ItemObject->Stack(SingleSplit);
		bResult &= bHasOverflow;
		// @gdemers validate output for the swap operation last executed.
		bResult &= (ItemObject->GetRuntimeCount() == StackBounds);
		bResult &= (SingleSplit->GetRuntimeCount() == FMath::CeilToInt(ItemObject->GetRuntimeCount() * 1.5) - StackBounds);
	}

	{
		// @gdemers test serializing split objects back to disk, and read from them.
		FStorageQualifierContextArgs Params;
		Params.PrivateItemIds = InventoryComponent->PrivateItemIds;
		Params.StoragePositionBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_POSITION_ENCODING_BIT_RANGE);
		Params.StorageIdBounds = UAVVMOnlineEncodingUtils::GetRangeAsBitMask(GET_ITEM_ID_ENCODING_BIT_RANGE);
		Params.CurrentStoragePosition = INDEX_NONE;
		Params.CurrentStorageId = INDEX_NONE;

		// @gdemers encode new storage position into UItemObject based on inventory latest layout.
		UItemObjectUtils::QualifyStorage(InventoryComponent.Get(), Params, SingleSplit);

		InventoryComponent->Items.Add(SingleSplit);
		InventoryComponent->CheckDisk();

		// @gdemers get-set file from disk caching all inventory providers representation.
		const FStringView FileContent = UInventoryFileHelper::Static_GetSetFileContent();

		const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(this);
		const FString OutInventoryProviders = UInventoryUtils::GetInventoryProviderById(FileContent.GetData(), TargetUniqueId);

		if (IsValid(SingleSplit))
		{
			// @gdemers item types share the same ItemId, as such, our only way of differentiating is through storage position.
			const int32 NonShiftedPrivateItemId = UItemObjectUtils::GetPrivateItemId(SingleSplit);
			const int32 NonShiftedStoragePosition = UItemObjectUtils::FilterStoragePosition(NonShiftedPrivateItemId);
			const int32 NewPrivateItemId = UInventoryUtils::GetItemStoragePosition(OutInventoryProviders, {}, NonShiftedStoragePosition);
			const int32 NewItemStackCount = UAVVMOnlineEncodingUtils::DecodeInt32(NewPrivateItemId, GET_ITEM_COUNT_ENCODING_BIT_RANGE, GET_ITEM_COUNT_ENCODING_RSHIFT);
			bResult &= (NewPrivateItemId != INDEX_NONE);
			bResult &= (NewItemStackCount == SingleSplit->GetRuntimeCount());
		}
	}

	return bResult;
}

void AAutomatedTestInventoryActor::OnRequestCompleted()
{
	// @gdemers Our async process is complete. All resources were loaded!
	ForceCompletion();
}
