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

#include <imgui.h>

#include "AVVM.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMSettings.h"
#include "DataRegistrySubsystem.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsModule.h"
#include "GameplayTagsSettings.h"
#include "Cheats/AVVMCheatData.h"
#include "Containers/StringFwd.h"
#include "Engine/AssetManager.h"
#include "HAL/FileManagerGeneric.h"
#include "ProfilingDebugging/CountersTrace.h"

// @gdemers for tracing how frequently these are being rebuilt if ever it becomes a problem!
TRACE_DECLARE_INT_COUNTER(ComboTagRebuild, TEXT("Combo Tag Rebuild Count"));
TRACE_DECLARE_INT_COUNTER(ComboRegistryRebuild, TEXT("Combo Registry Rebuild Count"));

void UAVVMCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Registering %s"), *GetName());

	// @gdemers hard reset between PIE sessions as the CheatExtension wasnt in memory
	// when editor changed was applied! for Runtime, we listen to the Module event so if a GFP is adding data registries or tags, we can process.
	bHasRegistriesChanged = true;
	bHasTagChanged = true;

	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this);
	IGameplayTagsModule::OnGameplayTagTreeChanged.AddUObject(this, &UAVVMCheatExtension::OnGameplayTagTreeChanged);

	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (IsValid(DataRegistrySubsystem))
	{
		// @gdemers upon activating a new GFP_DataRegistry, the subsystem will broadcast this event! (each time)
		DataRegistrySubsystem->OnSubsystemInitialized().AddUObject(this, &UAVVMCheatExtension::OnDataRegistrySubsystemChanged);
	}
}

void UAVVMCheatExtension::RemovedFromCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Unregistering %s"), *GetName());

	FAVVMDebuggerModule::Get().GetDebuggerContext().RemoveDescriptor(this);
	IGameplayTagsModule::OnGameplayTagTreeChanged.RemoveAll(this);

	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (IsValid(DataRegistrySubsystem))
	{
		DataRegistrySubsystem->OnSubsystemInitialized().RemoveAll(this);
	}

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

void UAVVMCheatExtension::Draw()
{
	if (ImGui::CollapsingHeader("Cheats"))
	{
		ImGui::BeginGroup();

		static int32 CurrentTagChannelIndex = 0;
		ImGui::Combo("Tag Channel",
		             &CurrentTagChannelIndex,
		             LazyGatherTagChannels(bHasTagChanged));

		static int32 CurrentRegistryIdIndex = 0;
		ImGui::Combo("Payload Registry Id",
		             &CurrentRegistryIdIndex,
		             LazyGatherRegistryIds(bHasRegistriesChanged));

		if (ImGui::Button("Notify"))
		{
			// @gdemers caching return value from Lazy*Function isnt possible. the null terminate character split the single character array into a singular entry when assigned to char*.
			const FString Channel = GetIndexedString(LazyGatherTagChannels(bHasTagChanged), CurrentTagChannelIndex);
			const FString Payload = GetIndexedString(LazyGatherRegistryIds(bHasRegistriesChanged), CurrentRegistryIdIndex);
			NotifyChannelWithPayload(Channel, Payload);
		}

		ImGui::EndGroup();
	}
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

inline const char* UAVVMCheatExtension::LazyGatherTagChannels(bool& bForceGathering) const
{
	static TAnsiStringBuilder<512> StringBuilder;
	static bool bWasInitialized = false;

	if (bWasInitialized && !bForceGathering)
	{
		return *StringBuilder;
	}

	TRACE_BOOKMARK(TEXT("UAVVMCheatExtension.LazyGatherTagChannels"));
	TRACE_COUNTER_INCREMENT(ComboTagRebuild);

	TArray<const FGameplayTagSource*> OutTagSources;
	// @gdemers notes : FindTagsWithSource doesn't get the path of the asset correctly. The impl details act on the TagSource->ConfigFileName (which is what I care about and
	// is the correct path) and append it.
	// Because they append the ConfigFileName path, it prevents retrieval of the tag list for the given Tag source.
	// UGameplayTagsManager::Get().FindTagsWithSource(TagSourcePath, OutTags);
	// Alternative, retrieve ALL tags under a /Tags directory, which is more intrusive than the previous approach!
	UGameplayTagsManager::Get().FindTagSourcesWithType(EGameplayTagSourceType::TagList, OutTagSources);

	StringBuilder.Reset();
	for (int32 i = 0; i < OutTagSources.Num(); ++i)
	{
		if (OutTagSources[i] == nullptr)
		{
			continue;
		}

		const TObjectPtr<UGameplayTagsList>& TagListObject = OutTagSources[i]->SourceTagList;
		for (const FGameplayTagTableRow& TagTableRow : TagListObject->GameplayTagList)
		{
			const FString TagString = TagTableRow.Tag.ToString();
			const TArray<TCHAR> CharArray = TagString.GetCharArray();
			StringBuilder.Append(CharArray);
			StringBuilder.Append("\0"/*enforce null termination between entries*/);
		}
	}

	bWasInitialized = true;
	bForceGathering = false;
	// @gdemers operator* and ToString enforce null termination
	return *StringBuilder/*last entry will be \0\0 as expected by ImGui::Combo*/;
}

inline const char* UAVVMCheatExtension::LazyGatherRegistryIds(bool& bForceGathering) const
{
	static TAnsiStringBuilder<512> StringBuilder;
	static bool bWasInitialized = false;

	if (bWasInitialized && !bForceGathering)
	{
		return *StringBuilder;
	}

	TRACE_BOOKMARK(TEXT("UAVVMCheatExtension.LazyGatherRegistryIds"));
	TRACE_COUNTER_INCREMENT(ComboRegistryRebuild);

	auto* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(DataRegistrySubsystem))
	{
		return nullptr;
	}

	const UDataRegistry* Registry = DataRegistrySubsystem->GetRegistryForType(UAVVMSettings::GetCheatRegistryType());
	if (!IsValid(Registry))
	{
		return nullptr;
	}

	// @gdemers TODO UDataRegistry::GetAllSourceItems is wrapped in #if WITH_EDITOR only preprocessor
	// find an alternative so we can use it in non-shipping build!
	TArray<FDataRegistrySourceItemId> SourceItems;
	Registry->GetAllSourceItems(SourceItems);

	StringBuilder.Reset();

	for (int32 i = 0; i < SourceItems.Num(); ++i)
	{
		const FString TagString = SourceItems[i].ItemId.ItemName.ToString();
		const TArray<TCHAR> CharArray = TagString.GetCharArray();
		StringBuilder.Append(CharArray);
		StringBuilder.Append("\0");
	}

	bWasInitialized = true;
	bForceGathering = false;
	return *StringBuilder;
}

inline FString UAVVMCheatExtension::GetIndexedString(const char* ConcatString, const int32 Index) const
{
	int32 ReverseCount = Index;

	const char* Head = ConcatString;
	while (*Head && ReverseCount > 0)
	{
		Head += strlen(Head) + 1;
		--ReverseCount;
	}

	return FString(Head);
}

void UAVVMCheatExtension::OnDataRegistrySubsystemChanged()
{
	bHasRegistriesChanged = true;
}

void UAVVMCheatExtension::OnGameplayTagTreeChanged()
{
	bHasTagChanged = true;
}
