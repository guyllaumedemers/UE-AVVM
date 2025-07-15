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
#include "ActorFenceComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMReplicatedTagComponent.h"
#include "FenceManagerSubsystem.h"

void UActorFenceComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" Class Instance to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorFenceComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter = Outer;
	TryRaise();
}

void UActorFenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UFenceManagerSubsystem::Static_UnregisterFence(this, this);

	UAVVMReplicatedTagComponent* NewReplicatedTagComponent = ReplicatedTagComponent.Get();
	if (IsValid(NewReplicatedTagComponent))
	{
		NewReplicatedTagComponent->OnReplicatedTagChanged.RemoveAll(this);
	}

	ReplicatedTagComponent.Reset();

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" Class Instance to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UActorFenceComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

void UActorFenceComponent::TryRaise()
{
	if (!ensureAlwaysMsgf(!FenceRequirements.IsEmpty(),
	                      TEXT("UActorFenceComponent is attempting to Raise a Fence on an Empty Ruleset!")))
	{
		return;
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UAVVMReplicatedTagComponent* NewReplicatedTagComponent = ReplicatedTagComponent.Get();
	if (!IsValid(NewReplicatedTagComponent))
	{
		NewReplicatedTagComponent = Outer->GetComponentByClass<UAVVMReplicatedTagComponent>();
		ReplicatedTagComponent = NewReplicatedTagComponent;
	}

	if (!ensureAlwaysMsgf(IsValid(NewReplicatedTagComponent),
	                      TEXT("Missing UAVVMReplicatedTagComponent! Outer doesn't reference the required Component!")))
	{
		return;
	}

	const bool bDoesMeetAllRequirements = NewReplicatedTagComponent->HasAllExact(FenceRequirements);
	if (!bDoesMeetAllRequirements)
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Adding Fence Requirements \"%s\"."),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *FenceRequirements.ToString())

		NewReplicatedTagComponent->OnReplicatedTagChanged.AddUniqueDynamic(this, &UActorFenceComponent::OnReplicatedTagChanged);
		UFenceManagerSubsystem::Static_RegisterFence(this, this);
	}
}

void UActorFenceComponent::TryLower()
{
	if (!ensureAlwaysMsgf(!FenceRequirements.IsEmpty(),
	                      TEXT("UActorFenceComponent is attempting to Raise a Fence on an Empty Ruleset!")))
	{
		return;
	}

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UAVVMReplicatedTagComponent* NewReplicatedTagComponent = ReplicatedTagComponent.Get();
	if (!IsValid(NewReplicatedTagComponent))
	{
		NewReplicatedTagComponent = Outer->GetComponentByClass<UAVVMReplicatedTagComponent>();
		ReplicatedTagComponent = NewReplicatedTagComponent;
	}

	const bool bDoesMeetAllRequirements = NewReplicatedTagComponent->HasAllExact(FenceRequirements);
	if (bDoesMeetAllRequirements)
	{
		BP_Execute();

		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Removing Fence Requirements \"%s\"."),
		       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
		       *FenceRequirements.ToString())

		UFenceManagerSubsystem::Static_UnregisterFence(this, this);
		NewReplicatedTagComponent->OnReplicatedTagChanged.RemoveAll(this);
	}
}

void UActorFenceComponent::OnReplicatedTagChanged(const FGameplayTagContainer& NewTags)
{
	TryLower();
}

void UActorFenceComponent::ForceLowering() const
{
	UAVVMReplicatedTagComponent* TagComponent = ReplicatedTagComponent.Get();
	if (IsValid(TagComponent))
	{
		TagComponent->Append(FenceRequirements);
	}
}
