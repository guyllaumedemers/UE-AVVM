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
#include "FastTransactionObject.h"

#include "AVVMLogger.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMToolkitModule.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Payloads/AVVMPrimitivePayloads.h"

FLightweightTransactionObject::FLightweightTransactionObject(const FUniqueNetIdRepl& NewPlayerUniqueId)
	: PlayerUniqueId(NewPlayerUniqueId)
{
	PlayerTransactions.OnGameplayTagStackObjectChanged.Clear();
	PlayerTransactions.OnGameplayTagStackObjectRemoved.Clear();
	PlayerTransactions.OnGameplayTagStackObjectChanged.AddRaw(this, &FLightweightTransactionObject::OnGameplayTagStackObjectChanged);
	PlayerTransactions.OnGameplayTagStackObjectRemoved.AddRaw(this, &FLightweightTransactionObject::OnGameplayTagStackObjectRemoved);
}

void FLightweightTransactionObject::PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("New FLightweightTransactionObject::%s Detected!"),
	                *PlayerUniqueId.ToString());

	auto* FastArraySerializer = StaticCast<FLightweightTransactionObjectFastArray*>(const_cast<FFastArraySerializer*>(&InArraySerializer));
	if (ensureAlwaysMsgf(FastArraySerializer != nullptr, TEXT("Failed to cast to owning type.")))
	{
		auto*& OutValue = FastArraySerializer->FastLookup.FindOrAdd(PlayerUniqueId);
		OutValue = this;
	}
}

void FLightweightTransactionObject::PreReplicatedRemove(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("Removing FLightweightTransactionObject::%s Detected!"),
	                *PlayerUniqueId.ToString());

	auto* FastArraySerializer = StaticCast<FLightweightTransactionObjectFastArray*>(const_cast<FFastArraySerializer*>(&InArraySerializer));
	if (ensureAlwaysMsgf(FastArraySerializer != nullptr, TEXT("Failed to cast to owning type.")) && FastArraySerializer->FastLookup.Contains(PlayerUniqueId))
	{
		FastArraySerializer->FastLookup.Remove(PlayerUniqueId);
	}
}

bool FLightweightTransactionObject::operator==(const FLightweightTransactionObject& Rhs) const
{
	return (PlayerTransactions.GetGameplayTagStackObjects() == Rhs.PlayerTransactions.GetGameplayTagStackObjects());
}

void FLightweightTransactionObject::OnGameplayTagStackObjectChanged(const FGameplayTag& Tag, const int32 NewValue) const
{
	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = Tag;
	ContextArgs.Payload = FAVVMNotificationPayload::Make<FAVVMInt32Payload>(NewValue);
	ContextArgs.Target = GetSetPlayerState(GEngine);

	UAVVMNotificationSubsystem::Static_BroadcastChannel(GEngine, ContextArgs);
}

void FLightweightTransactionObject::OnGameplayTagStackObjectRemoved(const FGameplayTag& Tag) const
{
	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = Tag;
	ContextArgs.Payload = {};
	ContextArgs.Target = GetSetPlayerState(GEngine);

	UAVVMNotificationSubsystem::Static_BroadcastChannel(GEngine, ContextArgs);
}

const APlayerState* FLightweightTransactionObject::GetSetPlayerState(const UObject* WorldContextObject) const
{
	if (!PlayerState.IsValid())
	{
		PlayerState = UGameplayStatics::GetPlayerStateFromUniqueNetId(WorldContextObject, PlayerUniqueId);
	}

	return PlayerState.Get();
}

int32 FLightweightTransactionObjectFastArray::Modify(const FUniqueNetIdRepl& PlayerUniqueId,
                                                     const FGameplayTag& Tag,
                                                     const int32 DeltaValue,
                                                     const bool bHasAuthority)
{
	FLightweightTransactionObject** SearchResult = FastLookup.Find(PlayerUniqueId);
	if ((SearchResult == nullptr) && bHasAuthority)
	{
		MarkArrayDirty();

		auto& Value = PerPlayerTransactions.Add_GetRef(FLightweightTransactionObject{PlayerUniqueId});
		Value.PlayerTransactions.Modify(Tag, DeltaValue, bHasAuthority);
		FastLookup.Add(PlayerUniqueId, &Value);
	}
	else if (SearchResult == nullptr)
	{
		AVVM_LOGGER_LOG(LogToolkit,
		                GEngine,
		                GEngine,
		                TEXT("Attempting to access %s from Client before initial FLightweightTransactionObject replication."),
		                *Tag.ToString());
	}
	else if ((*SearchResult) != nullptr)
	{
		if (bHasAuthority)
		{
			MarkItemDirty(**SearchResult);
		}

		return (*SearchResult)->PlayerTransactions.Modify(Tag, DeltaValue, bHasAuthority);
	}

	return DeltaValue;
}
