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
#include "AVVMNotificationSubsystem.h"

#include "AVVM.h"
#include "Archetypes/AVVMPresenter.h"
#include "Engine/World.h"

// @gdemers extern symbol for global access to custom LLM_tag
extern FLLMTagDeclaration LLMTagDeclaration_AVVMTag;

TInstancedStruct<FAVVMNotificationPayload> FAVVMNotificationPayload::Empty = TInstancedStruct<FAVVMNotificationPayload>();

UScriptStruct* TBaseStructure<FAVVMNotificationPayload>::Get()
{
	return FAVVMNotificationPayload::StaticStruct();
}

bool UAVVMNotificationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (IsValid(World))
	{
		return !World->IsNetMode(NM_DedicatedServer);
	}

	return false;
}

void UAVVMNotificationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogUI, Log, TEXT("UAVVMNotificationSubsystem::Initialize. Running On Client."));
}

UAVVMNotificationSubsystem* UAVVMNotificationSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	if (IsValid(World))
	{
		return UWorld::GetSubsystem<UAVVMNotificationSubsystem>(World);
	}
	else
	{
		return nullptr;
	}
}

void UAVVMNotificationSubsystem::Static_UnregisterObserver(const UObject* WorldContextObject,
                                                           const FAVVMObserverContextArgs& ObserverContext)
{
	if (!IsValid(WorldContextObject))
	{
		return;
	}

	const auto* OwningActor = Cast<AActor>(WorldContextObject);
	if (!IsValid(OwningActor))
	{
		OwningActor = WorldContextObject->GetTypedOuter<const AActor>();
	}

	if (!ensureAlwaysMsgf(IsValid(OwningActor),
	                      TEXT("UObject provided isnt owned by a valid AActor. This may not be the correct place to unbind to this subsystem.")))
	{
		return;
	}

	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(WorldContextObject);
	if (IsValid(AVVMNotificationSubsystem))
	{
		FAVVObserversFilteringMechanism& FilteringMechanism = AVVMNotificationSubsystem->ObserversFilteringMechanism;
		FilteringMechanism.Unregister(OwningActor, ObserverContext.ChannelTag);
	}
}

void UAVVMNotificationSubsystem::Static_RegisterObserver(const UObject* WorldContextObject,
                                                         const FAVVMObserverContextArgs& ObserverContext)
{
	if (!IsValid(WorldContextObject))
	{
		return;
	}

	const auto* OwningActor = Cast<AActor>(WorldContextObject);
	if (!IsValid(OwningActor))
	{
		OwningActor = WorldContextObject->GetTypedOuter<const AActor>();
	}

	if (!ensureAlwaysMsgf(IsValid(OwningActor),
	                      TEXT("UObject provided isnt owned by a valid AActor. This may not be the correct place to bind to this subsystem.")))
	{
		return;
	}

	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(WorldContextObject);
	if (IsValid(AVVMNotificationSubsystem))
	{
		FAVVObserversFilteringMechanism& FilteringMechanism = AVVMNotificationSubsystem->ObserversFilteringMechanism;
		FilteringMechanism.Register(OwningActor, ObserverContext.ChannelTag, ObserverContext.Callback);
	}
}

void UAVVMNotificationSubsystem::Static_BroadcastChannel(const UObject* WorldContextObject,
                                                         const FAVVMNotificationContextArgs& NotificationContext)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(WorldContextObject);
	if (IsValid(AVVMNotificationSubsystem))
	{
		FAVVObserversFilteringMechanism& FilteringMechanism = AVVMNotificationSubsystem->ObserversFilteringMechanism;
		FilteringMechanism.Broadcast(NotificationContext);
	}
}

void UAVVMNotificationSubsystem::Static_ExecuteDeferredNotifications(const UObject* WorldContextObject)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(WorldContextObject);
	if (IsValid(AVVMNotificationSubsystem))
	{
		FAVVObserversFilteringMechanism& FilteringMechanism = AVVMNotificationSubsystem->ObserversFilteringMechanism;
		FilteringMechanism.ExecuteDeferredNotifications();
	}
}

#if WITH_AUTOMATION_TESTS
int32 UAVVMNotificationSubsystem::Static_GetChannelCount(const UObject* WorldContextObject,
                                                         const FGameplayTag& ChannelTag)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(WorldContextObject);
	if (!IsValid(AVVMNotificationSubsystem))
	{
		return INDEX_NONE;
	}

	const bool bDoesContains = AVVMNotificationSubsystem->ObserversFilteringMechanism.TagToObservers.Contains(ChannelTag);
	if (!bDoesContains)
	{
		return INDEX_NONE;
	}

	const FAVVMObservers& Observers = AVVMNotificationSubsystem->ObserversFilteringMechanism.TagToObservers[ChannelTag];
	return Observers.Observers.Num();
}

int32 UAVVMNotificationSubsystem::Static_GetChannelsCount(const UObject* WorldContextObject)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(WorldContextObject);
	if (IsValid(AVVMNotificationSubsystem))
	{
		return AVVMNotificationSubsystem->ObserversFilteringMechanism.TagToObservers.Num();
	}
	else
	{
		return INDEX_NONE;
	}
}
#endif

UAVVMNotificationSubsystem::FAVVMObservers::~FAVVMObservers()
{
	Observers.Reset();
}

void UAVVMNotificationSubsystem::FAVVMObservers::Unregister(const AActor* Target)
{
	Observers.Remove(Target);
}

void UAVVMNotificationSubsystem::FAVVMObservers::Register(const AActor* Target,
                                                          const FAVVMOnChannelNotifiedSingleCastDelegate& Callback)
{
	FAVVMOnChannelNotifiedSingleCastDelegate& OutResult = Observers.FindOrAdd(Target);
	OutResult = Callback;
}

void UAVVMNotificationSubsystem::FAVVMObservers::BroadcastAll(const TInstancedStruct<FAVVMNotificationPayload>& Payload) const
{
	for (const auto& [Actor, Callback] : Observers)
	{
		Callback.ExecuteIfBound(Payload);
	}
}

void UAVVMNotificationSubsystem::FAVVMObservers::Broadcast(const AActor* Target,
                                                           const TInstancedStruct<FAVVMNotificationPayload>& Payload) const
{
	const FAVVMOnChannelNotifiedSingleCastDelegate* SearchResult = Observers.Find(Target);
	if (ensureAlwaysMsgf(SearchResult != nullptr, TEXT("FAVVMObservers::Broadcast provided with invalid Target Actor")))
	{
		SearchResult->ExecuteIfBound(Payload);
	}
}

bool UAVVMNotificationSubsystem::FAVVMObservers::IsEmpty() const
{
	return Observers.IsEmpty();
}

UAVVMNotificationSubsystem::FAVVObserversFilteringMechanism::~FAVVObserversFilteringMechanism()
{
	PendingRequests.Reset();
	TagToObservers.Reset();
}

void UAVVMNotificationSubsystem::FAVVObserversFilteringMechanism::Unregister(const AActor* Target,
                                                                             const FGameplayTag& ChannelTag)
{
	FAVVMObservers* SearchResult = TagToObservers.Find(ChannelTag);
	if (SearchResult == nullptr)
	{
		return;
	}

	SearchResult->Unregister(Target);

	if (SearchResult->IsEmpty())
	{
		TagToObservers.Remove(ChannelTag);
	}
}

void UAVVMNotificationSubsystem::FAVVObserversFilteringMechanism::Register(const AActor* Target,
                                                                           const FGameplayTag& ChannelTag,
                                                                           const FAVVMOnChannelNotifiedSingleCastDelegate& Callback)
{
	FAVVMObservers& SearchResult = TagToObservers.FindOrAdd(ChannelTag);
	SearchResult.Register(Target, Callback);
}

void UAVVMNotificationSubsystem::FAVVObserversFilteringMechanism::Broadcast(const FAVVMNotificationContextArgs& NotificationContext)
{
	const FAVVMObservers* SearchResult = TagToObservers.Find(NotificationContext.ChannelTag);
	if (SearchResult == nullptr)
	{
		UE_LOG(LogUI,
		       Log,
		       TEXT("Deferring Tag Channel \"%s\" Notification on \"%s\"."),
		       *NotificationContext.ChannelTag.ToString(),
		       NotificationContext.Target.IsValid() ? *NotificationContext.Target->GetName() : TEXT("All Registered Actors"));

		PendingRequests.Add(NotificationContext);
		return;
	}

	const TInstancedStruct<FAVVMNotificationPayload>& Payload = NotificationContext.Payload;
	const AActor* Target = NotificationContext.Target.Get();
	if (!IsValid(Target))
	{
		SearchResult->BroadcastAll(Payload);
	}
	else
	{
		SearchResult->Broadcast(Target, Payload);
	}
}

void UAVVMNotificationSubsystem::FAVVObserversFilteringMechanism::ExecuteDeferredNotifications()
{
	TArray<FAVVMNotificationContextArgs> OldPendingRequests = MoveTemp(PendingRequests);
	for (auto Iterator = OldPendingRequests.CreateIterator(); Iterator; ++Iterator)
	{
		const FAVVMNotificationContextArgs OldCtxArgs = *Iterator;
		Iterator.RemoveCurrentSwap();
		Broadcast(OldCtxArgs);
	}
}
