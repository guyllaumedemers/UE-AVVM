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
#include "Transaction.h"

#include "AVVMUtilityFunctionLibrary.h"
#include "DoesTransactionProviderSupportIdentifier.h"
#include "TransactionDeveloperSettings.h"
#include "TransactionFactoryUtils.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void UTransaction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTransaction, TargetId);
	DOREPLIFETIME(UTransaction, TransactionType);
	DOREPLIFETIME(UTransaction, Payload);
}

bool UTransaction::IsSupportedForNetworking() const
{
	return true;
}

#if UE_WITH_IRIS
void UTransaction::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicaitonFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

bool UTransaction::DoesExactMatch(const FString& NewTargetId, const ETransactionType NewTransactionType) const
{
	return (TargetId.Equals(NewTargetId) && (TransactionType == NewTransactionType));
}

bool UTransaction::DoesPartialMatch(const FString& NewTargetId) const
{
	return TargetId.Equals(NewTargetId);
}

FString UTransaction::ToString() const
{
	FStringFormatNamedArguments Args;
	Args.Add(TEXT("Instigator"), FStringFormatArg{InstigatorId});
	Args.Add(TEXT("Target"), FStringFormatArg{TargetId});
	Args.Add(TEXT("TransactionType"), FStringFormatArg{EnumToString(TransactionType)});
	Args.Add(TEXT("Payload"), FStringFormatArg{Payload});
	return FString::Format(TEXT("Instigator:{Instigator}\nTarget:{Target}\nTransactionType:{TransactionType}\nValue:\n\t{Payload}.\n"), Args);
}

TInstancedStruct<FTransactionPayload> UTransaction::GetValue() const
{
	const TInstancedStruct<FTransactionFactoryImpl> FactoryImpl = UTransactionDeveloperSettings::GetFactoryImpl(TransactionType);
	return UTransactionFactoryUtils::CreatePayloadFromString(FactoryImpl, Payload);
}

FString UTransaction::GetUniqueId(const AActor* NewTarget)
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

		const bool bImplements = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IDoesTransactionProviderSupportIdentifier,
		                                                                                              UDoesTransactionProviderSupportIdentifier>(StatisticOwner);
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

void UTransaction::operator()(const AActor* NewInstigator,
                              const AActor* NewTarget,
                              const ETransactionType NewTransactionType,
                              const FString& NewPayload)
{
	InstigatorId = UTransaction::GetUniqueId(NewInstigator);
	TargetId = UTransaction::GetUniqueId(NewTarget);
	TransactionType = NewTransactionType;
	Payload = NewPayload;
}
