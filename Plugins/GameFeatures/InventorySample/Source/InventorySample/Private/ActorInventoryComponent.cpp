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

#include "AVVMCharacter.h"
#include "AVVMGameplayUtils.h"
#include "AVVMGameSession.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMReplicatedTagComponent.h"
#include "AVVMScopedUtils.h"
#include "AVVMUtils.h"
#include "InventoryProvider.h"
#include "InventorySample.h"
#include "InventorySettings.h"
#include "InventoryUtils.h"
#include "ItemObject.h"
#include "NativeGameplayTags.h"
#include "NonReplicatedLoadoutObject.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Data/ItemDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"
#include "UI/InventoryNotificationPayload.h"

TRACE_DECLARE_INT_COUNTER(UActorInventoryComponent_InstanceCounter, TEXT("Inventory Component Instance Counter"));

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_NOTIFICATION_PICKUP_ITEM, "InventorySample.Item.Notification.Pickup");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_NOTIFICATION_SWAP_ITEM, "InventorySample.Item.Notification.Swap");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_NOTIFICATION_DROP_ITEM, "InventorySample.Item.Notification.Drop");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_STORAGE_NOENTRY, "InventorySample.Item.Storage.NoEntry");
UE_DEFINE_GAMEPLAY_TAG(TAG_INVENTORY_ITEM_DROPPABLE, "InventorySample.Item.Droppable");

TArray<int32> FInventoryDataResolverHelper::GetElementDependencies(const UObject* Outer, const int32 ElementId) const
{
	if (!IsValid(Outer))
	{
		return TArray<int32>{};
	}

	TArray<int32> OutResults;

	const auto* Character = Cast<AAVVMCharacter>(Outer);
	if (IsValid(Character))
	{
		OutResults = AAVVMGameSession::Static_GetPlayerInventoryItems(Outer->GetWorld(), ElementId/*calling Player UniqueId*/);
	}
	else
	{
		// @gdemers we may attempt retrieving the inventory for an NPC actor
		// (or any other actor type) that are defined in backend.
		OutResults = AAVVMGameSession::Static_GetActorInventoryItems(Outer->GetWorld(), ElementId/*calling Actor UniqueId*/);
	}

	return OutResults;
}

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
	if (Outer->HasAuthority())
	{
		if (bShouldAsyncLoadOnBeginPlay)
		{
			RequestItems(Outer);
		}

		auto* TagComponent = Outer->GetComponentByClass<UAVVMReplicatedTagComponent>();
		if (IsValid(TagComponent))
		{
			TagComponent->OnReplicatedTagChanged.AddUniqueDynamic(this, &UActorInventoryComponent::OnOuterTagChanged);
		}
	}
#endif

	bool bDoesSupportLoadout = false;

	auto* Character = Cast<AAVVMCharacter>(Outer);
	if (IsValid(Character))
	{
		bDoesSupportLoadout = true;
	}

	if (!NonReplicatedLoadoutClass.IsNull() && bDoesSupportLoadout)
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
	PrivateItemIds.Reset();

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

#if WITH_SERVER_CODE
	if (Outer->HasAuthority())
	{
		auto* TagComponent = Outer->GetComponentByClass<UAVVMReplicatedTagComponent>();
		if (IsValid(TagComponent))
		{
			TagComponent->OnReplicatedTagChanged.RemoveAll(this);
		}
	}
#endif

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
		// @gdemers using the Outer UniqueId, request the associated entry on backend {FAVVMPlayerProfile.UniqueId} or otherwise if
		// your backend tracks Actors differently. See AAVVMGameSession::Static_GetPlayerProfileId for profile access.
		// https://miro.com/app/board/uXjVJYdFx2Y=/?share_link_id=345226885183
		// Note : I would suggest using Data Asset to define the scheme of gameplay actors that don't evolve over time, and
		// keep the microservice request to be specific to your player account/character.
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

const TInstancedStruct<FAVVMDataResolverHelper>& UActorInventoryComponent::GetInventoryDataResolverHelper()
{
	static auto Helper = FAVVMDataResolverHelper::Make<FInventoryDataResolverHelper>();
	return Helper;
}

TInstancedStruct<FExecutionContextRule> UActorInventoryComponent::GetDropRule() const
{
	return FExecutionContextRule::Make<FDropRule>();
}

TInstancedStruct<FExecutionContextRule> UActorInventoryComponent::GetPickupRule() const
{
	return FExecutionContextRule::Make<FPickupRule>();
}

TInstancedStruct<FExecutionContextRule> UActorInventoryComponent::GetSwapRule() const
{
	return FExecutionContextRule::Make<FSwapRule>();
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
	return Compare.HasAnyExact(ComponentStateTags);
}

bool UActorInventoryComponent::HasExactMatch(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(ComponentStateTags);
}

void UActorInventoryComponent::Drop(UItemObject* PendingDropItemObject)
{
	const auto Ctx = FExecutionContextParams::Make<FDropContextParams>(PendingDropItemObject);
	const auto Rule = GetDropRule();
	const bool bWasSuccess = CanExecute(Ctx, Rule);
	if (bWasSuccess)
	{
		if ((GetOwnerRole() == ROLE_Authority))
		{
			OnDrop(PendingDropItemObject);
		}
		else
		{
			Server_Drop(PendingDropItemObject);
		}
	}

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_INVENTORY_NOTIFICATION_DROP_ITEM,
		                                        GetTypedOuter<APlayerController>(),
		                                        GetTypedOuter<AActor>(),
		                                        FAVVMNotificationPayload::Make<FInventoryNotificationPayload>(PendingDropItemObject, nullptr, bWasSuccess));
	}
}

void UActorInventoryComponent::Pickup(UItemObject* PendingPickupItemObject)
{
	const auto Ctx = FExecutionContextParams::Make<FPickupContextParams>(PendingPickupItemObject);
	const auto Rule = GetPickupRule();
	const bool bWasSuccess = CanExecute(Ctx, Rule);
	if (bWasSuccess)
	{
		if ((GetOwnerRole() == ROLE_Authority))
		{
			OnPickup(PendingPickupItemObject);
		}
		else
		{
			Server_Pickup(PendingPickupItemObject);
		}
	}

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_INVENTORY_NOTIFICATION_PICKUP_ITEM,
		                                        GetTypedOuter<APlayerController>(),
		                                        GetTypedOuter<AActor>(),
		                                        FAVVMNotificationPayload::Make<FInventoryNotificationPayload>(PendingPickupItemObject, nullptr, bWasSuccess));
	}
}

void UActorInventoryComponent::Swap(UItemObject* SrcItemObject, UItemObject* DestItemObject)
{
	const auto Ctx = FExecutionContextParams::Make<FSwapContextParams>(SrcItemObject, DestItemObject);
	const auto Rule = GetSwapRule();
	const bool bWasSuccess = CanExecute(Ctx, Rule);
	if (bWasSuccess)
	{
		if ((GetOwnerRole() == ROLE_Authority))
		{
			OnSwap(SrcItemObject, DestItemObject);
		}
		else
		{
			Server_Swap(SrcItemObject, DestItemObject);
		}
	}

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(this,
		                                        TAG_INVENTORY_NOTIFICATION_SWAP_ITEM,
		                                        GetTypedOuter<APlayerController>(),
		                                        GetTypedOuter<AActor>(),
		                                        FAVVMNotificationPayload::Make<FInventoryNotificationPayload>(SrcItemObject, DestItemObject, bWasSuccess));
	}
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

	const EItemSrcType ItemSrcType = IInventoryProvider::Execute_GetItemSrcType(Outer);
	const bool bIsNone = EnumHasAnyFlags(ItemSrcType, EItemSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("IHasItemCollection::GetItemSrcType is None. Check if it was properly overriden.")))
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
		if (!IsValid(NewItem))
		{
			continue;
		}

		// @gdemers initialize our ItemObject runtime representation. This integer encapsulate
		// information about storage, position within storage, item stack count, etc...
		int32 PrivateItemId = INDEX_NONE;

		const bool bIsItemSrcStatic = EnumHasAnyFlags(ItemSrcType, EItemSrcType::Static);
		if (bIsItemSrcStatic)
		{
			PrivateItemId = UItemObjectUtils::RuntimeInitStaticItem(Outer,
			                                                        PrivateItemIds,
			                                                        NewItem);
		}
		else
		{
			PrivateItemId = UItemObjectUtils::RuntimeInitOnlineItem(Outer,
			                                                        PrivateItemIds,
			                                                        UActorInventoryComponent::GetInventoryDataResolverHelper(),
			                                                        NewItem);
		}

		if (ensureAlwaysMsgf(PrivateItemId != INDEX_NONE,
		                     TEXT("Couldn't initialize Item with a valid ItemId.")))
		{
			PrivateItemIds.AddUnique(PrivateItemId);

			AddReplicatedSubObject(NewItem);
			Items.Add(NewItem);
		}
	}

	// @gdemers allow initialization of loadout object based held items. 
	OnRep_ItemCollectionChanged(OldItems);

	// @gdemers we have just initialized our component with new entries. validate our bounds between sessions.
	CheckBounds();

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

void UActorInventoryComponent::OnDrop(UItemObject* ItemObject)
{
	if (!IsValid(ItemObject))
	{
		return;
	}

	const bool bDoesContains = Items.Contains(ItemObject);
	if (!ensureAlwaysMsgf(bDoesContains, TEXT("Item no longer exist. Possible racing condition.")))
	{
		return;
	}

	TArray<UItemObject*> OldItems = Items;

	MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, Items, this);
	RemoveReplicatedSubObject(ItemObject);
	Items.Remove(ItemObject);

	OnRep_ItemCollectionChanged(OldItems);

	// @gdemers validate inventory bounds between removal, and remove any flags applied due to possible previous overflow.
	CheckBounds();

	// @gdemers update backend representation of our inventory.
	CheckBackend();
	
	// @gdemers handle invalidating the storage bits of the item encoding.
	UItemObjectUtils::NullifyStorage(ItemObject);

	// @gdemers Handle actor creation + UItemObject binding to World Actor created for
	// later retrieval during pickup.
	UItemObjectUtils::SpawnWorldItemActor(GetWorld(), ItemObject);
}

void UActorInventoryComponent::OnPickup(UItemObject* ItemObject)
{
	if (!IsValid(ItemObject))
	{
		return;
	}

	const TObjectPtr<UItemObject>* SearchResult = Items.FindByPredicate([InputItem = ItemObject](const UItemObject* SrcItem)
	{
		return IsValid(SrcItem) && SrcItem->CanStack(InputItem);
	});

	// @gdemers hard limit set by bit encoding to possible storage positions, and Ids.
	static constexpr int32 StoragePositionBounds = (1 << GET_ITEM_POSITION_ENCODING_BIT_RANGE);
	static constexpr int32 StorageIdBounds = (1 << GET_ITEM_ID_ENCODING_BIT_RANGE);
	bool bDoesStackOverflow = false;

	// @gdemers an entry already exist for us to support stacking.
	if ((SearchResult != nullptr) && IsValid(*SearchResult))
	{
		// @gdemers our stack was too big to fit the whole set, require inventory expansion.
		bDoesStackOverflow = (*SearchResult)->Stack(ItemObject);
	}
	else
	{
		FStorageQualifierContextArgs Params;
		Params.PrivateItemIds = PrivateItemIds;
		Params.StoragePositionBounds = StoragePositionBounds;
		Params.StorageIdBounds = StorageIdBounds;
		Params.CurrentStoragePosition = INDEX_NONE;
		Params.CurrentStorageId = INDEX_NONE;

		// @gdemers encode new storage position into UItemObject based on inventory available layout.
		UItemObjectUtils::QualifyStorage(this, Params, ItemObject);

		MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, Items, this);
		AddReplicatedSubObject(ItemObject);
		Items.Add(ItemObject);

		// @gdemers validate inventory bounds following addition.
		CheckBounds();
	}

	bool bHasAvailableEntries = true;
	if (bDoesStackOverflow)
	{
		// @gdemers get storage information from the item onto which we attempted stacking, but generated an overflow from.
		int32 TargetStoragePosition = (*SearchResult)->GetStoragePosition();
		int32 TargetStorageId = (*SearchResult)->GetStorageId();

		// @gdemers check if there is still unique entries available within our bounds.
		static const auto StorageFullTags = FGameplayTagContainer(TAG_INVENTORY_STORAGE_NOENTRY);
		// @gdemers add new entry in inventory with remaining stack count.
		// Note : UItemObject::Stack already handled updating the internal count for the existing slot but the remainder
		// held by the UItemObject may be greater than a new entry, so we need to split accordingly.
		MARK_PROPERTY_DIRTY_FROM_NAME(UActorInventoryComponent, Items, this);
		const int32 NumSplits = UItemObjectUtils::GetNumSplits(ItemObject);
		for (int32 i = 0; i < NumSplits; ++i)
		{
			bHasAvailableEntries = !HasPartialMatch(StorageFullTags);
			if (!bHasAvailableEntries)
			{
				break;
			}

			UItemObject* NewItemObjectEntry = UItemObjectUtils::SplitObject(this, ItemObject);
			if (IsValid(NewItemObjectEntry))
			{
				FStorageQualifierContextArgs Params;
				Params.PrivateItemIds = PrivateItemIds;
				Params.StoragePositionBounds = StoragePositionBounds;
				Params.StorageIdBounds = StorageIdBounds;
				Params.CurrentStoragePosition = TargetStoragePosition;
				Params.CurrentStorageId = TargetStorageId;

				// @gdemers encode new storage position into UItemObject based on inventory latest layout.
				UItemObjectUtils::QualifyStorage(this, Params, NewItemObjectEntry);

				AddReplicatedSubObject(NewItemObjectEntry);
				Items.Add(NewItemObjectEntry);

				// @gdemers update our last assigned storage information if ever we have more content to assign.
				TargetStoragePosition = NewItemObjectEntry->GetStoragePosition();
				TargetStorageId = NewItemObjectEntry->GetStorageId();
			}

			// @gdemers validate inventory bounds between addition to prevent overflow based
			// on design configuration for the owning Actor.
			CheckBounds();
		}
	}

	// @gdemers update backend representation of our inventory.
	CheckBackend();

	// @gdemers leave the owning actor in world for another player pickup.
	if (bDoesStackOverflow && !bHasAvailableEntries)
	{
		return;
	}

	// @gdemers destroy our owning interaction actor.
	UItemObjectUtils::DestroyWorldItemActor(ItemObject);

	// @gdemers reparent our item outer chain since we have destroyed our Pickup actor through our previous outer chain.
	ItemObject->Rename(nullptr, this);
}

void UActorInventoryComponent::OnSwap(UItemObject* SrcItemObject,
                                      UItemObject* DestItemObject)
{
	if (!IsValid(SrcItemObject) || !IsValid(DestItemObject))
	{
		return;
	}

	const int32 SrcStoragePosition = SrcItemObject->GetStoragePosition();
	const int32 SrcStorageId = SrcItemObject->GetStorageId();

	const int32 DestStoragePosition = DestItemObject->GetStoragePosition();
	const int32 DestStorageId = DestItemObject->GetStorageId();

	SrcItemObject->ModifyRuntimeStoragePosition(DestStoragePosition);
	SrcItemObject->ModifyRuntimeStorageId(DestStorageId);

	DestItemObject->ModifyRuntimeStoragePosition(SrcStoragePosition);
	DestItemObject->ModifyRuntimeStorageId(SrcStorageId);
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

bool UActorInventoryComponent::CanExecute(const TInstancedStruct<FExecutionContextParams>& Params,
                                          const TInstancedStruct<FExecutionContextRule>& Rule) const
{
	const auto* ContextRule = Rule.GetPtr<FExecutionContextRule>();
	if (!ensureAlwaysMsgf(ContextRule != nullptr, TEXT("FExecutionContextRule invalid.")))
	{
		return false;
	}

	const bool bPredicate = ContextRule->Predicate(this, Params);
	return bPredicate;
}

void UActorInventoryComponent::Server_Drop_Implementation(UItemObject* PendingDropItemObject)
{
	Drop(PendingDropItemObject);
}

void UActorInventoryComponent::Server_Pickup_Implementation(UItemObject* PendingDropItemObject)
{
	Pickup(PendingDropItemObject);
}

void UActorInventoryComponent::Server_Swap_Implementation(UItemObject* SrcItemObject, UItemObject* DestItemObject)
{
	Swap(SrcItemObject, DestItemObject);
}

void UActorInventoryComponent::CheckBackend() const
{
	const AActor* Outer = OwningOuter.Get();

	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")) ||
		!UAVVMUtils::IsNativeScriptInterfaceValid<const IAVVMResourceProvider>(Outer))
	{
		return;
	}

	// @gdemers target any actor type referencing this inventory component that may have a backend representation.
	// example : a shop with progression that tracks player purchases during gameplay. (Dead space does that!)
	const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on IAVVMResourceProvider::GetProviderUniqueId implementation."),
	                      *Outer->GetName()))
	{
		return;
	}

	const TArray<int32> OldDependencies = UAVVMOnlineBackendUtils::GetElementDependencies(Outer, TargetUniqueId, UActorInventoryComponent::GetInventoryDataResolverHelper());
	const TArray<int32> NewDependencies = UInventoryUtils::GetRuntimeUniqueIds(Items);

	const bool bAreSetIdentical = UAVVMOnlineBackendUtils::CompareSet(OldDependencies, NewDependencies);
	if (ensureAlwaysMsgf((false == bAreSetIdentical),
	                     TEXT("Attempting backend update on identical Sets.")))
	{
		const FString NewProfile = UInventoryUtils::ModifyProfile(this, TargetUniqueId, NewDependencies);
		UAVVMOnlineBackendUtils::Submit(this, TargetUniqueId, NewProfile);
	}
}

void UActorInventoryComponent::CheckBounds()
{
	TMap<int32/*StorageId*/, int32/*Count*/> OccupiedSlots;
	for (const UItemObject* Item : Items)
	{
		if (!IsValid(Item))
		{
			continue;
		}

		const int32 StorageId = Item->GetStorageId();
		int32& OutCount = OccupiedSlots.FindOrAdd(StorageId);
		++OutCount;
	}

	bool bHasStorageReachMaxCapacity = true;
	for (auto& [StorageId, Count] : OccupiedSlots)
	{
		bHasStorageReachMaxCapacity &= UItemObjectUtils::HasStorageReachMaxCapacity(this, StorageId, Count);
		if (!bHasStorageReachMaxCapacity)
		{
			break;
		}
	}

	static const auto StorageFullTags = FGameplayTagContainer(TAG_INVENTORY_STORAGE_NOENTRY);
	if (bHasStorageReachMaxCapacity)
	{
		ModifyRuntimeState(StorageFullTags, {});
	}
	else
	{
		ModifyRuntimeState({}, StorageFullTags);
	}
}

void UActorInventoryComponent::OnOuterTagChanged(const FGameplayTagContainer& NewTags)
{
	const bool bShouldDrop = NewTags.HasAnyExact(OuterDropConditionTags);
	if (!bShouldDrop)
	{
		return;
	}

	static const auto DroppableTagContainer = FGameplayTagContainer(TAG_INVENTORY_ITEM_DROPPABLE);
	const TArray<UItemObject*> PendingDropItems = Items.FilterByPredicate([Compare = DroppableTagContainer](const UItemObject* Item)
	{
		return IsValid(Item) && Item->DoesBehaviourHasPartialMatch(Compare);
	});

	for (UItemObject* PendingDropItem : PendingDropItems)
	{
		Drop(PendingDropItem);
	}
}

bool UActorInventoryUtils::CheckWeightOverflow(const UActorInventoryComponent* InventoryComponent,
											   const UItemObject* NewItemObject)
{
	if (!IsValid(InventoryComponent) || !IsValid(NewItemObject))
	{
		return false;
	}

	// TODO @gdemers retrieve ASC current Weight, and MaxWeight.
	// also retrieve the ItemActor weight, and compare to ensure we dont overflow.
	const auto* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(InventoryComponent->GetTypedOuter<AActor>());
	return true;
}
