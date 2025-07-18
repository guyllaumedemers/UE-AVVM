﻿//Copyright(c) 2025 gdemers
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

	DOREPLIFETIME(UInteraction, Target);
	DOREPLIFETIME(UInteraction, Instigator);
	DOREPLIFETIME(UInteraction, bIsInteractable);
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

bool UInteraction::DoesPartialMatch(const AActor* NewInstigator) const
{
	return IsValid(NewInstigator) && (Instigator == NewInstigator);
}

bool UInteraction::DoesExactMatch(const AActor* NewInstigator,
                                  const AActor* NewTarget) const
{
	return IsValid(NewTarget) && IsValid(NewInstigator) && (Target == NewTarget) && (Instigator == NewInstigator);
}

bool UInteraction::IsEqual(const UInteraction* Other) const
{
	return (Instigator == Other->Instigator) && (Target == Other->Target);
}

bool UInteraction::CanInteract() const
{
	return bIsInteractable;
}

void UInteraction::Lock()
{
	bIsInteractable = false;
}

void UInteraction::Unlock()
{
	bIsInteractable = true;
}

const AActor* UInteraction::GetTarget() const
{
	return Target.Get();
}

const AActor* UInteraction::GetInstigator() const
{
	return Instigator.Get();
}

void UInteraction::operator()(const AActor* NewInstigator,
                              const AActor* NewTarget)
{
	Instigator = NewInstigator;
	Target = NewTarget;
}
