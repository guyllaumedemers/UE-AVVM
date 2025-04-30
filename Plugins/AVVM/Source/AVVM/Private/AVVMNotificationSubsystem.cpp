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
#include "AVVMSettings.h"
#include "AVVMUtilityFunctionLibrary.h"
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

void UAVVMNotificationSubsystem::Static_BroadcastChannel(const FAVVMNotificationContextArgs& NotificationContext)
{
	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(NotificationContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		AVVMNotificationSubsystem->BroadcastChannel(NotificationContext);
	}
}

void UAVVMNotificationSubsystem::Static_UnregisterObserver(const FAVVMObserverContextArgs& ObserverContext)
{
	if (ObserverContext.bIsClassDefaultObject)
	{
		return;
	}

	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(ObserverContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		const TScriptInterface<IAVVMObserver> Presenter = ObserverContext.Observer;
		AVVMNotificationSubsystem->RemoveOrDestroy(Presenter->GetChannelTags(), Presenter);
	}
}

void UAVVMNotificationSubsystem::Static_RegisterObserver(const FAVVMObserverContextArgs& ObserverContext)
{
	if (ObserverContext.bIsClassDefaultObject)
	{
		return;
	}

	auto* AVVMNotificationSubsystem = UAVVMNotificationSubsystem::Get(ObserverContext.WorldContext);
	if (IsValid(AVVMNotificationSubsystem))
	{
		const TScriptInterface<IAVVMObserver> Presenter = ObserverContext.Observer;
		AVVMNotificationSubsystem->CreateOrAdd(Presenter->GetChannelTags(), Presenter);
	}
}

UAVVMNotificationSubsystem::FAVVMResolverContext::FAVVMResolverContext(const EAVVMObserverResolverFlag ResolverFlag)
{
	// TODO investigate!!
	// @gdemers FAVVMResolverContext may be created too often
	TRACE_BOOKMARK(TEXT("FAVVMResolverContext::FAVVMResolverContext()"));
	LLM_SCOPE_BYTAG(AVVMTag);

	// @gdemers having this static should be fairly safe, even in PIE, with multiple world, as this factory
	// only handle local creation of a context type that perform filtering behaviour.
	// we also benefit from lazy initialization of this entity.
	static TScriptInterface<IAVVMResolverFactoryImpl> GlobalResolver;

	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid<IAVVMResolverFactoryImpl>(GlobalResolver);
	if (!bIsValid)
	{
		const TSubclassOf<UObject>& ResolverClass = UAVVMSettings::GetFactoryResolverClass();
		GlobalResolver = NewObject<UObject>(ResolverClass);
	}

	Pimpl = GlobalResolver->Factory(ResolverFlag);
}

TArray<TScriptInterface<IAVVMObserver>> UAVVMNotificationSubsystem::FAVVMResolverContext::Filter(const FString& MatchRequirement,
                                                                                                 const TArray<TScriptInterface<IAVVMObserver>>& Observers) const
{
	const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid<IAVVMResolverExecutioner>(Pimpl);
	if (bIsValid)
	{
		return Pimpl->Filter(MatchRequirement, Observers);
	}
	else
	{
		return TArray<TScriptInterface<IAVVMObserver>>{};
	}
}

UAVVMNotificationSubsystem::FAVVMTagChannelObserverCollection::~FAVVMTagChannelObserverCollection()
{
	Observers.Empty();
}

void UAVVMNotificationSubsystem::FAVVMTagChannelObserverCollection::ResolveObservers(const FAVVMResolverContext& Context,
                                                                                     const FString& MatchRequirement,
                                                                                     TArray<TScriptInterface<IAVVMObserver>>& Out) const
{
	TRACE_BOOKMARK(TEXT("FTagChannelObserverCollection.ResolveObservers"));
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("Resolving Observers"));
	Out = Context.Filter(MatchRequirement, Observers);
}

void UAVVMNotificationSubsystem::FAVVMTagChannelObserverCollection::RemoveOrDestroy(const TScriptInterface<IAVVMObserver>& Observer)
{
	Observers.RemoveSwap(Observer);
}

void UAVVMNotificationSubsystem::FAVVMTagChannelObserverCollection::CreateOrAdd(const TScriptInterface<IAVVMObserver>& Observer)
{
	Observers.Add(Observer);
}

void UAVVMNotificationSubsystem::BroadcastChannel(const FAVVMNotificationContextArgs& NotificationContext) const
{
	const FAVVMTagChannelObserverCollection* SearchResult = TagChannels.Find(NotificationContext.ChannelTag);
	if (!ensure(SearchResult != nullptr))
	{
		return;
	}

	// @gdemers filter observers for the given channel. based on user requirements, we can filter the collection to restrict the message
	// to only broadcast for Observers that share the unique instance, or be more general and broadcast for a shared actor class.
	TArray<TScriptInterface<IAVVMObserver>> OutResult;
	SearchResult->ResolveObservers(FAVVMResolverContext(NotificationContext.ResolverFlag),
	                               NotificationContext.MatchRequirement,
	                               OutResult);

	for (auto Iterator{OutResult.CreateIterator()}; Iterator; ++Iterator)
	{
		const bool bIsValid = UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid<IAVVMObserver>(*Iterator);
		if (bIsValid)
		{
			(*Iterator)->Broadcast(NotificationContext.ChannelTag, NotificationContext.Payload);
		}
	}
}

void UAVVMNotificationSubsystem::RemoveOrDestroy(const FGameplayTagContainer& TagContainer,
                                                 const TScriptInterface<IAVVMObserver>& Observer)
{
	for (const FGameplayTag& Tag : TagContainer)
	{
		FAVVMTagChannelObserverCollection* ObserverCollection = TagChannels.Find(Tag);
		if (ensure(ObserverCollection != nullptr))
		{
			ObserverCollection->RemoveOrDestroy(Observer);
		}
	}
}

void UAVVMNotificationSubsystem::CreateOrAdd(const FGameplayTagContainer& TagContainer,
                                             const TScriptInterface<IAVVMObserver>& Observer)
{
	for (const FGameplayTag& Tag : TagContainer)
	{
		FAVVMTagChannelObserverCollection& ObserverCollection = TagChannels.FindOrAdd(Tag);
		ObserverCollection.CreateOrAdd(Observer);
	}
}
