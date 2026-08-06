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
#include "TransactionObject.h"

#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
#include "DoesTransactionProviderSupportIdentifier.h"
#include "NativeGameplayTags.h"
#include "TransactionSampleModule.h"
#include "TransactionSettings.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_TRANSACTION_NOTIFICATION, "TransactionSample.UIChannel.Notification.Transaction");

FTransactionObject::FTransactionObject(const FString& NewInstigatorId,
                                       const FString& NewTargetId,
                                       const ETransactionType NewTransactionType,
                                       const FString& NewPayload)
	: InstigatorId(NewInstigatorId),
	  TargetId(NewTargetId),
	  TransactionType(NewTransactionType),
	  Payload(NewPayload)
{
}

bool FTransactionObject::operator==(const FTransactionObject& Rhs) const
{
	return InstigatorId.Equals(Rhs.InstigatorId) &&
			TargetId.Equals(Rhs.TargetId) &&
			(TransactionType == Rhs.TransactionType) &&
			Payload.Equals(Rhs.Payload);
}

void FTransactionObject::PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
{
	// TODO @gdemers we may have to convert our UniqueNetId FString into actual ptr ref to
	// keep a handle on actors & allow api calls requiring WorldContextObject.
	AVVM_LOGGER_LOG(LogTransactionSample,
	                GEngine,
	                GEngine,
	                TEXT("New Transaction Detected! \r\n Value: %s."),
	                *UTransactionObjectUtils::ToString(*this));

	FAVVMNotificationContextArgs ContextArgs;
	ContextArgs.ChannelTag = TAG_TRANSACTION_NOTIFICATION;
	ContextArgs.Payload = UTransactionObjectUtils::GetValue(*this);
	ContextArgs.Target = nullptr;
	
	UAVVMNotificationSubsystem::Static_BroadcastChannel(GEngine, ContextArgs);
}

bool UTransactionObjectUtils::DoesExactMatch(const FTransactionObject& NewTransactionObject,
                                             const FString& NewTargetId,
                                             const ETransactionType NewTransactionType)
{
	return (NewTransactionObject.TargetId.Equals(NewTargetId) && (NewTransactionObject.TransactionType == NewTransactionType));
}

bool UTransactionObjectUtils::DoesPartialMatch(const FTransactionObject& NewTransactionObject,
                                               const FString& NewTargetId)
{
	return NewTransactionObject.TargetId.Equals(NewTargetId);
}

FString UTransactionObjectUtils::ToString(const FTransactionObject& NewTransactionObject)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Instigator"), NewTransactionObject.InstigatorId);
	JsonData->SetStringField(TEXT("Target"), NewTransactionObject.TargetId);
	JsonData->SetStringField(TEXT("TransactionType"), EnumToString(NewTransactionObject.TransactionType));
	JsonData->SetStringField(TEXT("Payload"), NewTransactionObject.Payload);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return JsonOutput;
	}

	return TEXT("Unknown");
}

TInstancedStruct<FTransactionPayload> UTransactionObjectUtils::GetValue(const FTransactionObject& NewTransactionObject)
{
	const TSubclassOf<UTransactionFactoryImpl> FactoryImpl = UTransactionSettings::GetFactoryImpl(NewTransactionObject.TransactionType);
	if (ensureAlwaysMsgf(IsValid(FactoryImpl),
	                     TEXT("Match not found. Missing FactoryImpl Class for Transaction Type \"%s\"."),
	                     EnumToString(NewTransactionObject.TransactionType)))
	{
		return UTransactionFactoryUtils::CreatePayloadFromString(FactoryImpl, NewTransactionObject.Payload);
	}
	else
	{
		return FTransactionPayload::Empty;
	}
}

FString UTransactionObjectUtils::GetUniqueId(const AActor* NewTarget)
{
	TFunction<void(const AActor* StatisticOwner, FString& OutUniqueId)> FindActorId;
	FindActorId = [&FindActorId](const AActor* StatisticOwner, FString& OutUniqueId)
	{
		if (!IsValid(StatisticOwner))
		{
			return;
		}

		const auto* NewPlayerState = Cast<APlayerState>(StatisticOwner);
		if (IsValid(NewPlayerState))
		{
			const FUniqueNetIdRepl& UniqueNetId = NewPlayerState->GetUniqueId();
			if (ensureAlwaysMsgf(UniqueNetId.IsValid(), TEXT("FUniqueNetIdRepl invalid!")))
			{
				OutUniqueId = UniqueNetId->ToString();
				return;
			}
		}

		const auto* Controller = Cast<AController>(StatisticOwner);
		if (IsValid(Controller))
		{
			const AActor* Target = IsValid(Controller->PlayerState) ? Cast<AActor>(Controller->PlayerState) : Cast<AActor>(Controller->GetPawn());
			FindActorId(Target, OutUniqueId);
			return;
		}

		const auto* Pawn = Cast<APawn>(StatisticOwner);
		if (IsValid(Pawn))
		{
			const auto* PlayerState = Cast<APlayerState>(Pawn->GetPlayerState());
			if (IsValid(PlayerState))
			{
				FindActorId(PlayerState, OutUniqueId);
				return;
			}
		}

		const bool bImplements = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UDoesTransactionProviderSupportIdentifier>(StatisticOwner);
		if (bImplements)
		{
			// @gdemers else, we care about statistics specific to inanimated world objects.
			OutUniqueId = IDoesTransactionProviderSupportIdentifier::Execute_GetProviderIdentifier(StatisticOwner);
		}
	};

	FString OutActorId;
	FindActorId(NewTarget, OutActorId);
	return OutActorId;
}

FTransactionObject UTransactionObjectUtils::MakeTransaction(const AActor* NewInstigator,
                                                            const AActor* NewTarget,
                                                            const ETransactionType NewTransactionType,
                                                            const FString& NewPayload)
{
	return FTransactionObject
	{
			UTransactionObjectUtils::GetUniqueId(NewInstigator),
			UTransactionObjectUtils::GetUniqueId(NewTarget),
			NewTransactionType,
			NewPayload
	};
}
