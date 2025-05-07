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
#include "Cheats/AVVMCheatExtension.h"

#include "AVVM.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMSettings.h"
#include "DataRegistrySubsystem.h"
#include "Cheats/AVVMCheatData.h"
#include "Engine/AssetManager.h"

void UAVVMCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Registering %s"), *GetName());
	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this, GetDescriptors());
}

void UAVVMCheatExtension::RemovedFromCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Unregistering %s"), *GetName());
	FAVVMDebuggerModule::Get().GetDebuggerContext().RemoveDescriptor(this);
	ClearAllStreamableHandle();
	ClearAllRequests();
}

void UAVVMCheatExtension::NotifyChannelWithPayload(const FString& TagChannel,
                                                   const FString& PayloadRegistryId)
{
	if (!ensureAlways(FGameplayTag::IsValidGameplayTagString(TagChannel)))
	{
		UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. Invalid Tag Channel!"), *GetName(), *TagChannel);
		return;
	}

	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. Failed!"), *GetName(), *TagChannel);
		return;
	}

	const auto SearchRegistryId = FDataRegistryId{UAVVMSettings::GetCheatRegistryType(), FName(PayloadRegistryId)};
	if (!ensureAlways(DataRegistrySubsystem->IsValidDataRegistryId(SearchRegistryId)))
	{
		UE_LOG(LogUI, Log, TEXT("Resource.Acquisition.%s. Failed!"), *SearchRegistryId.ToString());
		return;
	}

	const auto ChannelTag = FGameplayTag::RequestGameplayTag(FName(TagChannel));

	const TSharedPtr<FStreamableHandle>* SearchResult = StreamableHandles.Find(SearchRegistryId);
	if (SearchResult != nullptr)
	{
		UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. Progress Complete!"), *GetName(), *TagChannel);
		FAVVMNotificationContextArgs ContextArgs;
		ContextArgs.ChannelTag = ChannelTag;
		ContextArgs.WorldContextObject = this;
		ContextArgs.Payload = GetPayload(*SearchResult);
		UAVVMNotificationSubsystem::Static_BroadcastChannel(ContextArgs);
	}
	else
	{
		UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. In-progress..."), *GetName(), *TagChannel);
		PushRequest({SearchRegistryId, ChannelTag});

		FDataRegistryItemAcquiredCallback Callback;
		Callback.BindUObject(this, &UAVVMCheatExtension::OnRegistryIdAcquired);
		ensureAlwaysMsgf(DataRegistrySubsystem->AcquireItem(SearchRegistryId, Callback), TEXT("Delegate couldn't be schedule."));
	}
}

void UAVVMCheatExtension::NotifyChannelNoPayload(const FString& TagChannel)
{
	if (!ensureAlways(FGameplayTag::IsValidGameplayTagString(TagChannel)))
	{
		UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. Invalid Tag Channel!"), *GetName(), *TagChannel);
		return;
	}

	UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. Progress Complete!"), *GetName(), *TagChannel);
	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = FGameplayTag::RequestGameplayTag(FName(TagChannel));
	ContextArgs.WorldContextObject = this;
	ContextArgs.Payload = TInstancedStruct<FAVVMNotificationPayload>{};
	UAVVMNotificationSubsystem::Static_BroadcastChannel(ContextArgs);
}

void UAVVMCheatExtension::OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result)
{
	const bool bIsFullyLoaded = Result.Status == EDataRegistryAcquireStatus::AcquireFinished;
	if (!bIsFullyLoaded)
	{
		UE_LOG(LogUI, Log, TEXT("Resource.Acquisition.%s. Failed!"), *Result.ItemId.ToString());
		return;
	}

	const auto* CheatDataTableRow = Result.GetItem<FAVVMCheatDataTableRow>();
	check(CheatDataTableRow != nullptr /*unlikely to be invalid if we just loaded it, right!*/);

	FStreamableDelegate Callback;
	Callback.BindUObject(this, &UAVVMCheatExtension::OnSoftObjectAcquired);

	const auto LookupPaths = TArray<FSoftObjectPath>{CheatDataTableRow->CheatDataAsset.ToSoftObjectPath()};
	TSharedPtr<FStreamableHandle> OutStreamableHandle = UAssetManager::Get().LoadAssetList(LookupPaths, Callback);
	AddStreamableHandle(Result.ItemId, OutStreamableHandle);
}

void UAVVMCheatExtension::OnSoftObjectAcquired()
{
	// @gdemers since we dont know which user request was loaded, we have to look for all requests that are loaded, pending for load or ready
	// and execute those that are ready.
	for (auto Iterator{NotificationRequests.CreateIterator()}; Iterator; ++Iterator)
	{
		const FDataRegistryId& LookAtRegistryId = Iterator->Key;
		const FGameplayTag& LookAtChannelTag = Iterator->Value;

		const TSharedPtr<FStreamableHandle>* StreamableHandle = StreamableHandles.Find(LookAtRegistryId);
		if (!(StreamableHandle != nullptr && StreamableHandle->IsValid()))
		{
			Iterator.RemoveCurrentSwap();
			continue;
		}

		FStreamableHandle* Handle = StreamableHandle->Get();
		if (!Handle->HasLoadCompletedOrStalled())
		{
			continue;
		}

		UE_LOG(LogUI, Log, TEXT("%s invoked... Notify.Channel.%s. Progress Complete!"), *GetName(), *LookAtChannelTag.ToString());

		FAVVMNotificationContextArgs ContextArgs;
		ContextArgs.ChannelTag = LookAtChannelTag;
		ContextArgs.WorldContextObject = this;
		ContextArgs.Payload = GetPayload(*StreamableHandle);
		UAVVMNotificationSubsystem::Static_BroadcastChannel(ContextArgs);

		// @gdemers remove already broadcast request
		Iterator.RemoveCurrentSwap();
	}
}

void UAVVMCheatExtension::AddStreamableHandle(const FDataRegistryId& RegistryId,
                                              const TSharedPtr<FStreamableHandle> StreamableHandle)
{
	UE_LOG(LogUI, Log, TEXT("AddStreamableHandle."));
	StreamableHandles.Add(RegistryId, StreamableHandle);
}

void UAVVMCheatExtension::ClearAllStreamableHandle()
{
	UE_LOG(LogUI, Log, TEXT("ClearAllStreamableHandle."));
	StreamableHandles.Empty();
}

void UAVVMCheatExtension::PushRequest(const TPair<FDataRegistryId, FGameplayTag>& Request)
{
	UE_LOG(LogUI, Log, TEXT("PushRequest."));
	NotificationRequests.Push(Request);
}

void UAVVMCheatExtension::PopRequest()
{
	UE_LOG(LogUI, Log, TEXT("PopRequest."));
	NotificationRequests.Pop();
}

void UAVVMCheatExtension::ClearAllRequests()
{
	UE_LOG(LogUI, Log, TEXT("ClearAllRequests."));
	NotificationRequests.Empty();
}

TInstancedStruct<FAVVMCheatData> UAVVMCheatExtension::GetPayload(const TSharedPtr<FStreamableHandle> StreamableHandle)
{
	const auto* CheatDataAsset = StreamableHandle->GetLoadedAsset<UAVVMCheatDataAsset>();
	if (IsValid(CheatDataAsset))
	{
		return CheatDataAsset->GetData();
	}
	else
	{
		return {};
	}
}
