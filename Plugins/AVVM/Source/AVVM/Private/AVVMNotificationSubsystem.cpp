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

// @gdemers extern symbol for global access to custom LLM_tag
extern FLLMTagDeclaration LLMTagDeclaration_AVVMTag;

bool UAVVMNotificationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* PieOrGameWorld = Cast<UWorld>(Outer);
	if (IsValid(PieOrGameWorld))
	{
		const bool bIsGameClient = PieOrGameWorld->IsGameWorld() && !PieOrGameWorld->IsNetMode(NM_DedicatedServer);
		return bIsGameClient;
	}
	else
	{
		return false;
	}
}

void UAVVMNotificationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogUI, Log, TEXT("UAVVMNotificationSubsystem::Initialize. Running On Client."));
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

void UAVVMNotificationSubsystem::Static_UnregisterObserver(const FAVVMObserverContextArgs& ObserverContext,
                                                           const UObject* DelegateOwner)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(ObserverContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		AVVMNotificationSubsystem->RemoveOrDestroy(ObserverContext.ChannelTag, DelegateOwner);
	}
}

void UAVVMNotificationSubsystem::Static_RegisterObserver(const FAVVMObserverContextArgs& ObserverContext)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(ObserverContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		AVVMNotificationSubsystem->CreateOrAdd(ObserverContext.ChannelTag, ObserverContext.Callback);
	}
}

void UAVVMNotificationSubsystem::Static_BroadcastChannel(const FAVVMNotificationContextArgs& NotificationContext)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(NotificationContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		AVVMNotificationSubsystem->BroadcastChannel(NotificationContext);
	}
}

void UAVVMNotificationSubsystem::BroadcastChannel(const FAVVMNotificationContextArgs& NotificationContext) const
{
	const FAVVMOnChannelNotifiedMulticastDelegate* SearchResult = TagChannels.Find(NotificationContext.ChannelTag);
	if (ensure(SearchResult != nullptr))
	{
		SearchResult->Broadcast(NotificationContext.Payload);
	}
}

void UAVVMNotificationSubsystem::RemoveOrDestroy(const FGameplayTag& ChannelTag,
                                                 const UObject* DelegateOwner)
{
	FAVVMOnChannelNotifiedMulticastDelegate* MulticastDelegate = TagChannels.Find(ChannelTag);
	if (ensure(MulticastDelegate != nullptr))
	{
		MulticastDelegate->RemoveAll(DelegateOwner);
	}
}

void UAVVMNotificationSubsystem::CreateOrAdd(const FGameplayTag& ChannelTag,
                                             const FAVVMOnChannelNotifiedSingleCastDelegate& Callback)
{
	FAVVMOnChannelNotifiedMulticastDelegate& MulticastDelegate = TagChannels.FindOrAdd(ChannelTag);
	MulticastDelegate.AddUnique(Callback);
}
