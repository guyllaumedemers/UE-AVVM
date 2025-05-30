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
#include "Interaction.h"

#include "Net/UnrealNetwork.h"

void UInteraction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteraction, InteractionTarget);
	DOREPLIFETIME(UInteraction, InteractionInstigator);
	DOREPLIFETIME(UInteraction, bInUse);
}

bool UInteraction::IsSupportedForNetworking() const
{
	return true;
}

#if UE_WITH_IRIS
void UInteraction::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicaitonFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

bool UInteraction::DoesPartialMatch(const AActor* NewTarget) const
{
	return IsValid(NewTarget) && (InteractionTarget == NewTarget);
}

bool UInteraction::DoesMatch(const AActor* NewTarget,
                             const AActor* NewInstigator) const
{
	return IsValid(NewTarget) && IsValid(NewInstigator) && (InteractionTarget == NewTarget) && (InteractionInstigator == NewInstigator);
}

void UInteraction::operator()(const AActor* NewTarget,
                              const AActor* NewInstigator)
{
	InteractionInstigator = NewInstigator;
	InteractionTarget = NewTarget;
}
