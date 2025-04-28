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

#include "Archetypes/AVVMPresenter.h"

bool UAVVMNotificationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = GetWorld();
	const bool bIsGameClient = ensure(IsValid(World)) && World->IsGameWorld()
		                           ? World->GetNetMode() >= ENetMode::NM_Client
		                           : false;
	return bIsGameClient;
}

void UAVVMNotificationSubsystem::Deinitialize()
{
	Super::Deinitialize();
	TagChannels.Empty();
}

UAVVMNotificationSubsystem* UAVVMNotificationSubsystem::Get(const UWorld* WorldContext)
{
	return UWorld::GetSubsystem<UAVVMNotificationSubsystem>(WorldContext);
}

void UAVVMNotificationSubsystem::Static_BroadcastChannel(const FNotificationContextArgs& NotificationContext)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(NotificationContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		AVVMNotificationSubsystem->BroadcastChannel(NotificationContext.Payload, NotificationContext.ChannelTag);
	}
}

void UAVVMNotificationSubsystem::Static_UnregisterObserver(const FObserverContextArgs& ObserverContext)
{
	if (ObserverContext.bIsClassDefaultObject)
	{
		return;
	}

	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(ObserverContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		const TScriptInterface<IAVVMObserver> Presenter = ObserverContext.Observer;
		AVVMNotificationSubsystem->RemoveOrDestroy(Presenter, Presenter->GetChannelTags());
	}
}

void UAVVMNotificationSubsystem::Static_RegisterObserver(const FObserverContextArgs& ObserverContext)
{
	if (ObserverContext.bIsClassDefaultObject)
	{
		return;
	}

	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(ObserverContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		const TScriptInterface<IAVVMObserver> Presenter = ObserverContext.Observer;
		AVVMNotificationSubsystem->CreateOrAdd(Presenter, Presenter->GetChannelTags());
	}
}

UAVVMNotificationSubsystem::FTagChannelObserverCollection::~FTagChannelObserverCollection()
{
	Observers.Empty();
}

void UAVVMNotificationSubsystem::FTagChannelObserverCollection::ResolveObservers(const FInstancedStruct& Payload,
                                                                                 TArray<TScriptInterface<IAVVMObserver>>& Out) const
{
	// TODO Cast payload to type from which we can resolve OuterKey type
}

void UAVVMNotificationSubsystem::FTagChannelObserverCollection::RemoveOrDestroy(const TScriptInterface<IAVVMObserver>& Observer)
{
	Observers.Remove(Observer);
}

void UAVVMNotificationSubsystem::FTagChannelObserverCollection::CreateOrAdd(const TScriptInterface<IAVVMObserver>& Observer)
{
	Observers.Add(Observer);
}

void UAVVMNotificationSubsystem::BroadcastChannel(const FInstancedStruct& Payload,
                                                  const FGameplayTag& ChannelTag) const
{
	const FTagChannelObserverCollection* SearchResult = TagChannels.Find(ChannelTag);
	if (!ensure(SearchResult != nullptr))
	{
		return;
	}

	// @gdemers filter observers based on information given by payload so only the Observers using the same OuterKey
	// type will receive the notification for the given event. This allow to filter the event based on a specific Context Actor.
	TArray<TScriptInterface<IAVVMObserver>> OutResult;
	SearchResult->ResolveObservers(Payload, OutResult);

	for (auto Iterator{OutResult.CreateIterator()}; Iterator; ++Iterator)
	{
		if (ensure(Iterator->GetInterface() != nullptr
				&& Iterator->GetObject() != nullptr))
		{
			(*Iterator)->Broadcast(ChannelTag, Payload);
		}
	}
}

void UAVVMNotificationSubsystem::RemoveOrDestroy(const TScriptInterface<IAVVMObserver>& Observer,
                                                 const FGameplayTagContainer& TagContainer)
{
	for (const FGameplayTag& Tag : TagContainer)
	{
		FTagChannelObserverCollection* ObserverCollection = TagChannels.Find(Tag);
		if (ensure(ObserverCollection != nullptr))
		{
			ObserverCollection->RemoveOrDestroy(Observer);
		}
	}
}

void UAVVMNotificationSubsystem::CreateOrAdd(const TScriptInterface<IAVVMObserver>& Observer,
                                             const FGameplayTagContainer& TagContainer)
{
	for (const FGameplayTag& Tag : TagContainer)
	{
		FTagChannelObserverCollection& ObserverCollection = TagChannels.FindOrAdd(Tag);
		ObserverCollection.CreateOrAdd(Observer);
	}
}
