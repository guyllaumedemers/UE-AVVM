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

#include "AVVMGameplayUtils.h"
#include "AVVMScopedUtils.h"
#include "AVVMUtils.h"
#include "InventoryProvider.h"
#include "InventorySample.h"
#include "InventorySettings.h"
#include "ItemObject.h"
#include "NonReplicatedLoadoutObject.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

TRACE_DECLARE_INT_COUNTER(UActorInventoryComponent_InstanceCounter, TEXT("Inventory Component Instance Counter"));

UActorInventoryComponent::UActorInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);
	
	bReplicateUsingRegisteredSubObjectList = true;
}

void UActorInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UActorInventoryComponent, Items, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UActorInventoryComponent, ComponentStateTags, Params);
}

void UActorInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	QueueingMechanism = MakeShared<FItemSpawnerQueuingMechanism>();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UActorInventoryComponent_InstanceCounter);

	OwningOuter = Outer;

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorInventoryComponent::StaticClass()->GetName(),
	       *Outer->GetName());

#if WITH_SERVER_CODE
	if (bShouldAsyncLoadOnBeginPlay && Outer->HasAuthority())
	{
		RequestItems(Outer);
	}
#endif

	if (!NonReplicatedLoadoutClass.IsNull())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UActorInventoryComponent::OnLoadoutObjectRetrieved);
		LoadoutHandle = UAssetManager::Get().LoadAssetList({NonReplicatedLoadoutClass.ToSoftObjectPath()}, Callback);
	}
}

void UActorInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	QueueingMechanism.Reset();
	ItemHandleSystem.Reset();

	for (auto Iterator = Items.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorInventoryComponent::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter.Reset();
}

UActorInventoryComponent* UActorInventoryComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UActorInventoryComponent>() : nullptr;
}

void UActorInventoryComponent::RequestItems(const AActor* Outer)
{
	if (!IsValid(Outer) || !ensureAlwaysMsgf(Outer->HasAuthority(),
	                                         TEXT("Outer isn't Authoritative and cannot request loading items!")))
	{
		return;
	}

	const bool bResult = UAVVMUtils::IsBlueprintScriptInterfaceValid<UInventoryProvider>(Outer);
	if (!bResult)
	{
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, Items, this);
	for (auto Iterator = Items.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveReplicatedSubObject(Iterator->Get());
		Iterator.RemoveCurrentSwap();
	}

	const EItemSrcType ItemSrcType = IInventoryProvider::Execute_GetItemSrcType(Outer);
	const bool bIsNone = EnumHasAnyFlags(ItemSrcType, EItemSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IHasItemCollection::GetItemSrcType is None. Check if it was properly overriden.")))
	{
		return;
	}

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Executed from \"%s\". Requesting Items Type \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       EnumToString(ItemSrcType),
	       *Outer->GetName());

	const bool bIsItemSrcStatic = EnumHasAnyFlags(ItemSrcType, EItemSrcType::Static);
	if (bIsItemSrcStatic)
	{
		IInventoryProvider::Execute_RequestItemsFromDataAsset(Outer);
	}
	else
	{
		// @gdemers using the Outer UniqueId, request the associated entry on backend {FActorContent.UniqueId}.
		// https://miro.com/app/board/uXjVJYdFx2Y=/?share_link_id=345226885183
		// Note : see UInventoryUtils::GetAllRegistryIds(FActorContextProxy) to retrieve relevant Ids from Outer.
		IInventoryProvider::Execute_RequestItemsFromMicroService(Outer);
	}
}

void UActorInventoryComponent::SetupItemObjects(const TArray<UObject*>& NewResources)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                      TEXT("Attempting to load invalid Item set on Outer \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<FSoftObjectPath> DeferredItems;
	for (const UObject* Resource : NewResources)
	{
		const auto* ItemAsset = Cast<UItemDefinitionDataAsset>(Resource);
		if (!IsValid(ItemAsset))
		{
			continue;
		}

		if (!ItemAsset->CanAccessItem(ComponentStateTags, ComponentStateTags))
		{
			UE_LOG(LogInventorySample,
			       Log,
			       TEXT("Executed from \"%s\". Failed to Meet \"%s\" Requirements."),
			       IsServerOrClientString,
			       *ItemAsset->GetName());

			continue;
		}

		UE_LOG(LogInventorySample,
		       Log,
		       TEXT("Executed from \"%s\". New \"%s\" Recorded."),
		       IsServerOrClientString,
		       *ItemAsset->GetName());

		DeferredItems.Add(ItemAsset->GetItemObjectClass().ToSoftObjectPath());
	}

	if (!DeferredItems.IsEmpty())
	{
		FItemToken Token;
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UActorInventoryComponent::OnItemsRetrieved, Token);

		TSharedPtr<FStreamableHandle>& OutResult = ItemHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
	}
}

void UActorInventoryComponent::SetupItemActors(const TArray<UObject*>& NewResources)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                      TEXT("Attempting to load invalid Item set on Outer \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		return;
	}

	UItemObject* ItemObject = QueueingMechanism->PeekItem();
	if (!IsValid(ItemObject) || !ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                                              TEXT("Attempting to load invalid Actor Definition on Item \"%s\"."),
	                                              *ItemObject->GetName()))
	{
		return;
	}

	FOnRequestItemActorClassComplete Callback;
	Callback.BindDynamic(this, &UActorInventoryComponent::OnItemActorClassRetrieved);
	ItemObject->GetItemActorClassAsync(NewResources[0], Callback);
}

const TArray<UItemObject*>& UActorInventoryComponent::GetItems() const
{
	return Items;
}

void UActorInventoryComponent::ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags)
{
	ComponentStateTags.RemoveTags(RemovedTags);
	ComponentStateTags.AppendTags(AddedTags);

	MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, ComponentStateTags, this);
}

bool UActorInventoryComponent::HasPartialMatch(const FGameplayTagContainer& Compare) const
{
	return ComponentStateTags.HasAnyExact(Compare);
}

bool UActorInventoryComponent::HasExactMatch(const FGameplayTagContainer& Compare) const
{
	return ComponentStateTags.HasAllExact(Compare);
}

void UActorInventoryComponent::OnItemsRetrieved(FItemToken ItemToken)
{
	const TSharedPtr<FStreamableHandle>* OutResult = ItemHandleSystem.Find(ItemToken.UniqueId);
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	(*OutResult)->GetLoadedAssets(OutStreamableAssets);

	MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, Items, this);
	// @gdemers cache array before to invoke OnRep on server.
	TArray<UItemObject*> OldItems = Items;
	for (UObject* StreamableAsset : OutStreamableAssets)
	{
		auto* ItemObjectClass = Cast<UClass>(StreamableAsset);
		if (!IsValid(ItemObjectClass))
		{
			continue;
		}

		auto* NewItem = NewObject<UItemObject>(this, ItemObjectClass);
		AddReplicatedSubObject(NewItem);
		Items.Add(NewItem);
	}

	// @gdemers allow initialization of loadout object based held items. 
	OnRep_ItemCollectionChanged(OldItems);

	const bool bResult = UAVVMUtils::IsBlueprintScriptInterfaceValid<UInventoryProvider>(Outer);
	if (!bResult)
	{
		return;
	}

	const bool bDoesSupportSpawnOnLoad = IInventoryProvider::Execute_DoesSupportSpawnOnLoad(Outer);
	if (!bDoesSupportSpawnOnLoad)
	{
		return;
	}

	auto* ResourceManagerComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(Outer);
	if (!ensureAlwaysMsgf(IsValid(ResourceManagerComponent), TEXT("Outer doesn't return valid AVVMResourceManagerComponent!")))
	{
		return;
	}

	UItemObject* NewItem = nullptr;
	if (!Items.IsEmpty())
	{
		NewItem = Items.Last();
	}

	if (!IsValid(NewItem))
	{
		return;
	}

	// @gdemers handle spawning default object that are currently equipped
	const bool bIsItemEquipped = IInventoryProvider::Execute_IsItemEquipped(Outer, NewItem);
	if (bIsItemEquipped)
	{
		SpawnEquipItem(ResourceManagerComponent, NewItem);
	}
}

void UActorInventoryComponent::OnRep_ItemCollectionChanged(const TArray<UItemObject*>& OldItemObjects)
{
	auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	const int32 OldSize = OldItemObjects.Num();
	const int32 NewSize = Items.Num();

	FStringView SV;
	if (NewSize == OldSize)
	{
		SV = TEXT("has identical Size. We may have Swapped Items!");
	}
	else if (NewSize > OldSize)
	{
		SV = TEXT("has increased!");
	}
	else
	{
		SV = TEXT("has decreased!");
	}

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Executed from \"%s\". Item Collection modified on Outer \"%s\"! Collection %s"),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Outer->GetName(),
	       SV.GetData());

	if (IsValid(NonReplicatedLoadout))
	{
		NonReplicatedLoadout->HandleItemCollectionChanged(Items, OldItemObjects);
	}
}

void UActorInventoryComponent::SpawnEquipItem(UAVVMResourceManagerComponent* ResourceManagerComponent,
                                              UItemObject* NewItem)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(QueueingMechanism.IsValid(), TEXT("QueueingMechanism invalid!")))
	{
		return;
	}

	if (IsValid(NewItem))
	{
		NewItem->ModifyRuntimeState(FGameplayTagContainer{UInventorySettings::GetEquippedTag()}, {});
	}

	const auto RequestItemSpawning = [](TWeakObjectPtr<UActorInventoryComponent> NewActorInventoryComponent,
	                                    TWeakObjectPtr<UAVVMResourceManagerComponent> NewResourceManagerComponent,
	                                    TWeakObjectPtr<UItemObject> ItemToSpawn)
	{
		if (!ensureAlwaysMsgf(NewActorInventoryComponent.IsValid(), TEXT("WeakObjectPtr to Actor Inventory Component Invalid!")) ||
			!ensureAlwaysMsgf(NewResourceManagerComponent.IsValid(), TEXT("WeakObjectPtr to Resource Manager Component Invalid!")) ||
			!ensureAlwaysMsgf(ItemToSpawn.IsValid(), TEXT("WeakObjectPtr to ItemObject Invalid!")))
		{
			return;
		}

		const AActor* NewOuter = NewActorInventoryComponent->OwningOuter.Get();
		UE_LOG(LogInventorySample,
		       Log,
		       TEXT("Executed from \"%s\". Executing Spawn Item Request for \"%s\" on Outer \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(NewOuter).GetData(),
		       *ItemToSpawn->GetName(),
		       IsValid(NewOuter) ? *NewOuter->GetName() : TEXT("Unknown"));

		NewResourceManagerComponent->RequestAsyncLoading(ItemToSpawn->GetItemActorId(), {});
	};

	const auto NewRequest = FOnAsyncSpawnRequestDeferred::CreateWeakLambda(this,
	                                                                       RequestItemSpawning,
	                                                                       TWeakObjectPtr<UActorInventoryComponent>(this),
	                                                                       TWeakObjectPtr<UAVVMResourceManagerComponent>(ResourceManagerComponent),
	                                                                       TWeakObjectPtr<UItemObject>(NewItem));

	const bool bHasPendingRequest = QueueingMechanism->PushDeferredItem(NewItem, NewRequest);
	if (bHasPendingRequest)
	{
		UE_LOG(LogInventorySample,
		       Log,
		       TEXT("Executed from \"%s\". Spawn Item Request for \"%s\" was Deferred on Outer \"%s\"!"),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *NewItem->GetName(),
		       IsValid(Outer) ? *Outer->GetName() : TEXT("Unknown"));
	}
}

void UActorInventoryComponent::OnItemActorClassRetrieved(const UClass* NewActorClass,
                                                         const FSoftObjectPath& NewActorAttributeSetSoftObjectPath,
                                                         UItemObject* NewItemObject)
{
	const auto ScopedSafety = [](TSharedPtr<FItemSpawnerQueuingMechanism> NewQueueingMechanism)
	{
		if (ensureAlwaysMsgf(NewQueueingMechanism.IsValid(),
		                     TEXT("QueueingMechanism invalid!")))
		{
			NewQueueingMechanism->TryExecuteNextRequest(true);
		}
	};

	const auto Callback = FSimpleDelegate::CreateWeakLambda(this, ScopedSafety, QueueingMechanism);
	FAVVMScopedDelegate ScopedDelegate(Callback);

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer),
	                      TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(IsValid(NewItemObject),
	                      TEXT("Trying to access invalid UItemObject during initialization process.")))
	{
		return;
	}

	FItemActorSpawnContextArgs Params;
	Params.ActorClass = NewActorClass;
	Params.AttributeSetSoftObjectPath = NewActorAttributeSetSoftObjectPath;
	Params.Outer = const_cast<AActor*>(Outer);

	// @gdemers handle actor creation.
	NewItemObject->SpawnActor(Params);
}

void UActorInventoryComponent::OnLoadoutObjectRetrieved()
{
	if (!LoadoutHandle.IsValid())
	{
		return;
	}

	TArray<UObject*> OutResources;
	LoadoutHandle->GetLoadedAssets(OutResources);

	for (const UObject* Resource : OutResources)
	{
		const auto* NewLoadoutClass = Cast<UClass>(Resource);
		if (!IsValid(NewLoadoutClass))
		{
			continue;
		}

		NonReplicatedLoadout = NewObject<UNonReplicatedLoadoutObject>(this, NewLoadoutClass);
		// @gdemers server initialize loadout based on latest state cache.
		NonReplicatedLoadout->HandleItemCollectionChanged(Items, TArray<UItemObject*>());
	}
}

UActorInventoryComponent::FItemSpawnerQueuingMechanism::~FItemSpawnerQueuingMechanism()
{
	PendingSpawnRequests.Empty();
	QueuedItems.Empty();
}

bool UActorInventoryComponent::FItemSpawnerQueuingMechanism::PushDeferredItem(UItemObject* NewItem,
                                                                              const UActorInventoryComponent::FOnAsyncSpawnRequestDeferred& NewRequest)
{
	QueuedItems.Add(NewItem);
	PendingSpawnRequests.Add(NewRequest);
	TryExecuteNextRequest();
	return HasPendingRequest();
}

bool UActorInventoryComponent::FItemSpawnerQueuingMechanism::TryExecuteNextRequest(const bool bCanDequeueFrontItem)
{
	if (bCanDequeueFrontItem && !QueuedItems.IsEmpty())
	{
		QueuedItems.RemoveAtSwap(0);
	}

	if (!HasPendingRequest())
	{
		return true;
	}

	const bool bAreContainerEquals = QueuedItems.Num() == PendingSpawnRequests.Num();
	if (bAreContainerEquals)
	{
		// @gdemers TQueues dont support size...
		FOnAsyncSpawnRequestDeferred NextRequest = PendingSpawnRequests[0];
		PendingSpawnRequests.RemoveAtSwap(0);
		NextRequest.ExecuteIfBound();
	}

	return false;
}

bool UActorInventoryComponent::FItemSpawnerQueuingMechanism::HasPendingRequest() const
{
	return !PendingSpawnRequests.IsEmpty();
}

UItemObject* UActorInventoryComponent::FItemSpawnerQueuingMechanism::PeekItem() const
{
	TWeakObjectPtr<UItemObject> ItemObject = nullptr;
	if (!QueuedItems.IsEmpty())
	{
		ItemObject = QueuedItems[0].Get();
	}

	return ItemObject.Get();
}
