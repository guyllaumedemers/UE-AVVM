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

#include "Net/UnrealNetwork.h"

void UTransaction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTransaction, OwnerId);
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

bool UTransaction::DoesMatch(const FString& NewOwnerId, const ETransactionType NewTransactionType) const
{
	return (OwnerId.Equals(NewOwnerId) && (TransactionType == NewTransactionType));
}

void UTransaction::operator()(const FString& NewOwnerId,
                              const ETransactionType NewTransactionType,
                              const FString& NewPayload)
{
	OwnerId = NewOwnerId;
	TransactionType = NewTransactionType;
	Payload = NewPayload;

	const FString TransactionTypeString(EnumToString(NewTransactionType));
	UE_LOG(LogTemp,
	       Log,
	       TEXT("OwnerId: %s, Transaction type: %s, Payload: %s"),
	       *NewOwnerId,
	       *TransactionTypeString,
	       *NewPayload);
}
