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
#include "FastTransactionHistoryComponent.h"

#include "AVVMGameState.h"
#include "AVVMLogger.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMPlayerState.h"
#include "TransactionSampleModule.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Payloads/AVVMPrimitivePayloads.h"

UFastTransactionHistoryComponent::UFastTransactionHistoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = true;
}

void UFastTransactionHistoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UFastTransactionHistoryComponent, PerPlayerTransactions, Params);
}

void UFastTransactionHistoryComponent::BeginPlay()
{
	Super::BeginPlay();

	auto* Outer = GetTypedOuter<AAVVMGameState>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogTransactionSample,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(UFastTransactionHistoryComponent::StaticClass()));

	Outer->GetOnPlayerArrayResized().AddUObject(this, &UFastTransactionHistoryComponent::OnPlayerArrayResized);
	OwningOuter = Outer;
}

void UFastTransactionHistoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// @gdemers const_cast here so we prevent any edit to the cached outer at runtime.
	auto* Outer = const_cast<AAVVMGameState*>(OwningOuter.Get());
	if (IsValid(Outer))
	{
		Outer->GetOnPlayerArrayResized().RemoveAll(this);
	}

	PerPlayerTransactions.Reset();

	AVVM_LOGGER_LOG(LogTransactionSample,
	                OwningOuter.Get(),
	                OwningOuter.Get(),
	                TEXT("Removing %s."),
	                *GetNameSafe(UFastTransactionHistoryComponent::StaticClass()));
}

const TArray<FAVVMGameplayTagStackObjectFastArray>& UFastTransactionHistoryComponent::Static_GetFastTransactionObjects(const UObject* WorldContextObject)
{
	static const TArray<FAVVMGameplayTagStackObjectFastArray> Empty{};
	const auto* FastTransactionHistory = UFastTransactionHistoryComponent::GetActorComponent(WorldContextObject);
	return IsValid(FastTransactionHistory) ? FastTransactionHistory->GetFastTransactionObjects() : Empty;
}

FAVVMGameplayTagStackObjectFastArray UFastTransactionHistoryComponent::Static_GetFastLookupFromPlayerState(const UObject* WorldContextObject,
                                                                                                           const AAVVMPlayerState* PlayerState)
{
	static const FAVVMGameplayTagStackObjectFastArray Empty{};
	const auto* Result = IsValid(PlayerState) ? Static_GetFastLookup(WorldContextObject, PlayerState->GetUniqueId()) : nullptr;
	return (Result != nullptr) ? *Result : Empty;
}

FAVVMGameplayTagStackObject UFastTransactionHistoryComponent::Static_GetFastLookupTagStateFromPlayerState(const UObject* WorldContextObject,
                                                                                                          const AAVVMPlayerState* PlayerState,
                                                                                                          const FGameplayTag& Tag)
{
	static const FAVVMGameplayTagStackObject Empty{};
	const auto* Result = IsValid(PlayerState) ? Static_GetFastLookupTag(WorldContextObject, PlayerState->GetUniqueId(), Tag) : nullptr;
	return (Result != nullptr) ? *Result : Empty;
}

const FAVVMGameplayTagStackObjectFastArray* UFastTransactionHistoryComponent::Static_GetFastLookup(const UObject* WorldContextObject,
                                                                                                   const FUniqueNetIdRepl& UniqueNetId)
{
	const auto* FastTransactionHistory = UFastTransactionHistoryComponent::GetActorComponent(WorldContextObject);
	return IsValid(FastTransactionHistory) ? FastTransactionHistory->GetFastLookup(UniqueNetId) : nullptr;
}

const FAVVMGameplayTagStackObject* UFastTransactionHistoryComponent::Static_GetFastLookupTag(const UObject* WorldContextObject,
                                                                                             const FUniqueNetIdRepl& UniqueNetId,
                                                                                             const FGameplayTag& Tag)
{
	const auto* FastTransactionHistory = UFastTransactionHistoryComponent::GetActorComponent(WorldContextObject);
	return IsValid(FastTransactionHistory) ? FastTransactionHistory->GetFastLookupTag(UniqueNetId, Tag) : nullptr;
}

int32 UFastTransactionHistoryComponent::Static_ModifyTransaction(const UObject* WorldContextObject,
                                                                 const FUniqueNetIdRepl& PlayerUniqueId,
                                                                 const FGameplayTag& Tag,
                                                                 const int32 DeltaValue,
                                                                 const bool bHasAuthority)
{
	auto* FastTransactionHistory = UFastTransactionHistoryComponent::GetActorComponent(WorldContextObject);
	return IsValid(FastTransactionHistory) ? FastTransactionHistory->ModifyTransaction(PlayerUniqueId, Tag, DeltaValue, bHasAuthority) : INDEX_NONE;
}

UFastTransactionHistoryComponent* UFastTransactionHistoryComponent::GetActorComponent(const UObject* WorldContextObject)
{
	static TWeakObjectPtr<UFastTransactionHistoryComponent> TransactionHistory = nullptr;
	if (TransactionHistory.IsValid())
	{
		return TransactionHistory.Get();
	}

	const AGameStateBase* GameState = UGameplayStatics::GetGameState(WorldContextObject);
	if (IsValid(GameState))
	{
		TransactionHistory = GameState->GetComponentByClass<UFastTransactionHistoryComponent>();
	}

	return TransactionHistory.Get();
}

const TArray<FAVVMGameplayTagStackObjectFastArray>& UFastTransactionHistoryComponent::GetFastTransactionObjects() const
{
	return PerPlayerTransactions;
}

const FAVVMGameplayTagStackObjectFastArray* UFastTransactionHistoryComponent::GetFastLookup(const FUniqueNetIdRepl& UniqueNetId) const
{
	return FastLookup.Contains(UniqueNetId) ? *FastLookup.Find(UniqueNetId) : nullptr;
}

const FAVVMGameplayTagStackObject* UFastTransactionHistoryComponent::GetFastLookupTag(const FUniqueNetIdRepl& UniqueNetId, const FGameplayTag& Tag) const
{
	if (FastLookup.Contains(UniqueNetId))
	{
		const auto* SearchResult = FastLookup[UniqueNetId];
		return (SearchResult != nullptr) ? SearchResult->GetFastLookupTag(Tag) : nullptr;
	}
	else
	{
		return nullptr;
	}
}

int32 UFastTransactionHistoryComponent::ModifyTransaction(const FUniqueNetIdRepl& PlayerUniqueId,
                                                          const FGameplayTag& Tag,
                                                          const int32 DeltaValue,
                                                          const bool bHasAuthority)
{
	FAVVMGameplayTagStackObjectFastArray** SearchResult = FastLookup.Find(PlayerUniqueId);
	if ((SearchResult == nullptr) && bHasAuthority)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UFastTransactionHistoryComponent, PerPlayerTransactions, this);
		auto& Value = PerPlayerTransactions.Add_GetRef(FAVVMGameplayTagStackObjectFastArray{});
		Value.Modify(Tag, DeltaValue, bHasAuthority);
		FastLookup.Add(PlayerUniqueId, &Value);
	}
	else if (SearchResult == nullptr)
	{
		AVVM_LOGGER_LOG(LogTransactionSample,
		                OwningOuter.Get(),
		                OwningOuter.Get(),
		                TEXT("Attempting to access %s from Client before initial FLightweightTransactionObject replication."),
		                *Tag.ToString());
	}
	else if ((*SearchResult) != nullptr)
	{
		if (bHasAuthority)
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(UFastTransactionHistoryComponent, PerPlayerTransactions, this);
		}

		return (*SearchResult)->Modify(Tag, DeltaValue, bHasAuthority);
	}

	return DeltaValue;
}

void UFastTransactionHistoryComponent::OnPlayerArrayResized(AAVVMPlayerState* NewPlayerState,
                                                            const bool bExpanded)
{
	if (!ensureAlwaysMsgf(IsValid(NewPlayerState),
	                      TEXT("Failed to access PlayerState.")))
	{
		return;
	}

	const auto UniqueNetIdRef = NewPlayerState->GetUniqueId();
	if (!UniqueNetIdRef.IsValid() && !NewPlayerState->GetOnSetPlayerUniqueNetIdDelegate().IsBoundToObject(this))
	{
		NewPlayerState->GetOnSetPlayerUniqueNetIdDelegate().AddUObject(this, &UFastTransactionHistoryComponent::OnPlayerUniqueNetIdReady);
		return;
	}

	if (bExpanded)
	{
		OnPlayerUniqueNetIdReady(UniqueNetIdRef);
	}
	else if (bShouldReleasePlayerStatisticsOnEarlyLeave)
	{
		auto* SearchResult = FastLookup.Find(UniqueNetIdRef);
		if (ensureAlwaysMsgf(SearchResult != nullptr,
		                     TEXT("Player was never registered, or already removed.")))
		{
			(**SearchResult) = FAVVMGameplayTagStackObjectFastArray{};
			FastLookup.Remove(UniqueNetIdRef);
		}
	}
}

void UFastTransactionHistoryComponent::OnPlayerUniqueNetIdReady(const FUniqueNetIdRepl& UniqueNetIdRepl)
{
	ModifyTransaction(UniqueNetIdRepl,
	                  FGameplayTag::EmptyTag,
	                  0,
	                  (GetOwnerRole() == ROLE_Authority));
}

void UFastTransactionHistoryComponent::OnRep_OnPerPlayerTransactionsResize()
{
	for (auto& FastArray : PerPlayerTransactions)
	{
		if (!FastArray.OnGameplayTagStackObjectChanged.IsBoundToObject(this))
		{
			FastArray.OnGameplayTagStackObjectChanged.AddUObject(this, &UFastTransactionHistoryComponent::OnGameplayTagStackObjectChanged);
		}

		if (!FastArray.OnGameplayTagStackObjectRemoved.IsBoundToObject(this))
		{
			FastArray.OnGameplayTagStackObjectRemoved.AddUObject(this, &UFastTransactionHistoryComponent::OnGameplayTagStackObjectRemoved);
		}
	}
}

void UFastTransactionHistoryComponent::OnGameplayTagStackObjectChanged(const FAVVMGameplayTagStackObjectFastArray* FastArray,
                                                                       const FGameplayTag& Tag,
                                                                       const int32 NewValue) const
{
	const FUniqueNetIdRepl* UniqueNetIdRep = ReverseFastLookup.Find(FastArray);
	if (!ensureAlwaysMsgf(UniqueNetIdRep != nullptr,
	                      TEXT("Failed to retrieve valid UniqueNetId")))
	{
		return;
	}

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = Tag;
	ContextArgs.Payload = FAVVMNotificationPayload::Make<FAVVMInt32Payload>(NewValue);
	ContextArgs.Target = UGameplayStatics::GetPlayerStateFromUniqueNetId(this, *UniqueNetIdRep);

	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}

void UFastTransactionHistoryComponent::OnGameplayTagStackObjectRemoved(const FAVVMGameplayTagStackObjectFastArray* FastArray,
                                                                       const FGameplayTag& Tag) const
{
	const auto* UniqueNetIdRep = ReverseFastLookup.Find(FastArray);
	if (!ensureAlwaysMsgf(UniqueNetIdRep != nullptr,
	                      TEXT("Failed to retrieve valid UniqueNetId")))
	{
		return;
	}

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = Tag;
	ContextArgs.Payload = {};
	ContextArgs.Target = UGameplayStatics::GetPlayerStateFromUniqueNetId(this, *UniqueNetIdRep);

	UAVVMNotificationSubsystem::Static_BroadcastChannel(this, ContextArgs);
}
