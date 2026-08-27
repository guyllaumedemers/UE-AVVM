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
#include "InteractionObject.h"

#include "ActorInteractionComponent.h"
#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

FInteractionObject::FInteractionObject(const AActor* NewTarget,
                                       const AActor* NewInstigator,
                                       UActorInteractionComponent* NewInstigatorComponent)
	: Target(NewTarget),
	  Instigator(NewInstigator),
	  InstigatorComponent(NewInstigatorComponent)
{
}

bool FInteractionObject::DoesPartialMatch(const AActor* NewInstigator) const
{
	return !IsPendingKill() && IsValid(NewInstigator) && (Instigator == NewInstigator);
}

bool FInteractionObject::DoesExactMatch(const AActor* NewInstigator,
                                        const AActor* NewTarget) const
{
	return !IsPendingKill() && IsValid(NewTarget) && IsValid(NewInstigator) && (Target == NewTarget) && (Instigator == NewInstigator);
}

bool FInteractionObject::IsPendingKill() const
{
	return bIsPendingKill;
}

bool FInteractionObject::CanInteract() const
{
	return bIsInteractable;
}

void FInteractionObject::Lock()
{
	bIsInteractable = false;

	AVVM_LOGGER_LOG(LogGameplay,
	                Instigator.Get(),
	                Instigator.Get(),
	                TEXT("Lock interaction between %s, and %s"),
	                *GetNameSafe(Instigator.Get()),
	                *GetNameSafe(Target.Get()));
}

void FInteractionObject::Unlock()
{
	bIsInteractable = true;

	AVVM_LOGGER_LOG(LogGameplay,
	                Instigator.Get(),
	                Instigator.Get(),
	                TEXT("Unlock interaction between %s, and %s"),
	                *GetNameSafe(Instigator.Get()),
	                *GetNameSafe(Target.Get()));
}

const AActor* FInteractionObject::GetTarget() const
{
	return Target.Get();
}

const AActor* FInteractionObject::GetInstigator() const
{
	return Instigator.Get();
}

void FInteractionObject::SetPendingKill()
{
	bIsPendingKill = true;

	AVVM_LOGGER_LOG(LogGameplay,
	                Instigator.Get(),
	                Instigator.Get(),
	                TEXT("SetPendingKill between %s, and %s"),
	                *GetNameSafe(Instigator.Get()),
	                *GetNameSafe(Target.Get()));
}

void FInteractionObject::PreReplicatedRemove(const struct FFastArraySerializer& InArraySerializer)
{
	if (InstigatorComponent.IsValid())
	{
		InstigatorComponent->HandlePendingKillRecord(*this);
	}
}

void FInteractionObject::PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
{
	if (InstigatorComponent.IsValid())
	{
		InstigatorComponent->HandleNewRecord(*this);
	}
}

void FInteractionObject::PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer)
{
	if (InstigatorComponent.IsValid())
	{
		InstigatorComponent->HandlePendingKillRecord(*this);
	}
}

bool FInteractionObject::operator==(const FInteractionObject& Rhs) const
{
	return (Instigator == Rhs.Instigator) && (Target == Rhs.Target);
}
