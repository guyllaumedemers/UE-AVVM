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
#include "AVVMOnlinePlayerSubsystem.h"

#include "AVVMNotificationSubsystem.h"
#include "AVVMOnline.h"
#include "AVVMOnlinePlayerStringParser.h"
#include "AVVMOnlineSettings.h"
#include "NativeGameplayTags.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

bool UAVVMOnlinePlayerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (IsValid(World))
	{
		return !World->IsNetMode(NM_DedicatedServer);
	}

	return false;
}

void UAVVMOnlinePlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto* GameStateBase = UGameplayStatics::GetGameState(this);
	if (!IsValid(GameStateBase))
	{
		return;
	}

	FAVVMOnChannelNotifiedSingleCastDelegate Callback;
	Callback.BindDynamic(this, &UAVVMOnlinePlayerSubsystem::OnPlayerStateAddedOrRemoved);

	FAVVMObserverContextArgs ContextArgs;
	ContextArgs.ChannelTag = UAVVMOnlineSettings::GetPlayerStateChannelTag();
	ContextArgs.Callback = Callback;
	UAVVMNotificationSubsystem::Static_RegisterObserver(GameStateBase, ContextArgs);
}

void UAVVMOnlinePlayerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	auto* GameStateBase = UGameplayStatics::GetGameState(this);
	if (IsValid(GameStateBase))
	{
		FAVVMObserverContextArgs ContextArgs;
		ContextArgs.ChannelTag = UAVVMOnlineSettings::GetPlayerStateChannelTag();
		UAVVMNotificationSubsystem::Static_UnregisterObserver(GameStateBase, ContextArgs);
	}
}

FString UAVVMOnlinePlayerSubsystem::Static_GetPlayerProfile(const UWorld* World,
                                                            const int32 ProfileId)
{
	const auto* OnlinePlayerSubsystem = UAVVMOnlinePlayerSubsystem::Get(World);
	return IsValid(OnlinePlayerSubsystem) ? OnlinePlayerSubsystem->GetPlayerProfile(ProfileId) : FString();
}

FString UAVVMOnlinePlayerSubsystem::Static_GetPlayerPreset(const UWorld* World,
                                                           const int32 PresetId)
{
	const auto* OnlinePlayerSubsystem = UAVVMOnlinePlayerSubsystem::Get(World);
	return IsValid(OnlinePlayerSubsystem) ? OnlinePlayerSubsystem->GetPlayerPreset(PresetId) : FString();
}

TArray<int32> UAVVMOnlinePlayerSubsystem::Static_GetPlayerEquippedItems(const UWorld* World, const int32 ProfileId)
{
	const auto* OnlinePlayerSubsystem = UAVVMOnlinePlayerSubsystem::Get(World);
	return IsValid(OnlinePlayerSubsystem) ? OnlinePlayerSubsystem->GetPlayerEquippedItems(ProfileId) : TArray<int32>{};
}

UAVVMOnlinePlayerSubsystem* UAVVMOnlinePlayerSubsystem::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAVVMOnlinePlayerSubsystem>(World);
}

FString UAVVMOnlinePlayerSubsystem::GetPlayerProfile(const int32 ProfileId) const
{
	static constexpr FStringView Empty = TEXT("");

	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return Empty.GetData();
	}

	const bool bDoesContains = PlayerProfiles.Contains(ProfileId);
	if (bDoesContains)
	{
		return PlayerProfiles[ProfileId];
	}

	return Empty.GetData();
}

FString UAVVMOnlinePlayerSubsystem::GetPlayerPreset(const int32 PresetId) const
{
	static constexpr FStringView Empty = TEXT("");

	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
						  TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return Empty.GetData();
	}

	const bool bDoesContains = PlayerPresets.Contains(PresetId);
	if (bDoesContains)
	{
		return PlayerPresets[PresetId];
	}

	return Empty.GetData();
}

TArray<int32> UAVVMOnlinePlayerSubsystem::GetPlayerEquippedItems(const int32 ProfileId) const
{
	UAVVMOnlinePlayerStringParser* JsonParser = FAVVMOnlineModule::GetJsonParser_Player();
	if (!ensureAlwaysMsgf(IsValid(JsonParser),
	                      TEXT("FAVVMOnlineModule::GetJsonParser doesn't reference a valid parser.")))
	{
		return TArray<int32>{};
	}

	const FString ProfilePayload = GetPlayerProfile(ProfileId);
	if (ProfilePayload.IsEmpty())
	{
		return TArray<int32>{};
	}

	FAVVMPlayerProfile OutPlayerProfile;
	JsonParser->FromString(ProfilePayload, OutPlayerProfile);

	const FString PresetPayload = GetPlayerPreset(OutPlayerProfile.EquippedPresetId);
	if (PresetPayload.IsEmpty())
	{
		return TArray<int32>{};
	}

	FAVVMPlayerPreset OutPlayerPreset;
	JsonParser->FromString(PresetPayload, OutPlayerPreset);

	return OutPlayerPreset.EquippedItems;
}

void UAVVMOnlinePlayerSubsystem::OnPlayerStateAddedOrRemoved(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
}
