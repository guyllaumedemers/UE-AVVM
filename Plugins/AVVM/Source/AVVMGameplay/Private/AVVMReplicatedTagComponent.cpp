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
#include "AVVMReplicatedTagComponent.h"

#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UAVVMReplicatedTagComponent::UAVVMReplicatedTagComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);
}

void UAVVMReplicatedTagComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMReplicatedTagComponent, FilteredTags, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMReplicatedTagComponent, Flags, Params);
}

void UAVVMReplicatedTagComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Adding %s."),
	                *GetNameSafe(UAVVMReplicatedTagComponent::StaticClass()));

	OwningOuter = Outer;

	const FGameplayTagContainer CachePendingTags = PendingFlags;
	PendingFlags.Reset();
	ModifyRuntimeTags(CachePendingTags, {});
}

void UAVVMReplicatedTagComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UAVVMReplicatedTagComponent::StaticClass()));

	OwningOuter.Reset();
}

void UAVVMReplicatedTagComponent::ModifyFilteredTags(const FGameplayTagContainer& AddedTags,
                                                     const FGameplayTagContainer& RemovedTags)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(UAVVMReplicatedTagComponent, FilteredTags, this);
	if (AddedTags.IsValid())
	{
		FilteredTags.AppendTags(AddedTags);
	}

	if (RemovedTags.IsValid())
	{
		FilteredTags.RemoveTags(RemovedTags);
	}
}

void UAVVMReplicatedTagComponent::ModifyRuntimeTags(const FGameplayTagContainer& AddedTags,
                                                    const FGameplayTagContainer& RemovedTags)
{
	if (!OwningOuter.IsValid())
	{
		// @gdemers  There is cases in which we try appending as soon as the local player gets assigned a PC so to be able
		// to track adding primary game layout to the viewport resulting in the fence system lowering a condition.
		// The problem however is that Actor construction of the PC which initialize the component above only happens later after receiving the local player.
		PendingFlags.AppendTags(AddedTags);
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(UAVVMReplicatedTagComponent, Flags, this);
	const FGameplayTagContainer OldTags = Flags;

	if (AddedTags.IsValid())
	{
		Flags.AppendTags(AddedTags);
	}

	if (RemovedTags.IsValid())
	{
		Flags.RemoveTags(RemovedTags);
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                OwningOuter.Get(),
	                OwningOuter.Get(),
	                TEXT("Adding Tags %s. Removed Tags %s. Active Tags %s."),
	                *AddedTags.ToString(),
	                *RemovedTags.ToString(),
	                *Flags.ToString());

	OnRep_FlagsModified(OldTags);
}

bool UAVVMReplicatedTagComponent::HasAnyExactFilteredTags(const FGameplayTagContainer& Compare)
{
	return Compare.HasAnyExact(FilteredTags);
}

bool UAVVMReplicatedTagComponent::HasAllExactFilteredTags(const FGameplayTagContainer& Compare)
{
	return Compare.HasAllExact(FilteredTags);
}

bool UAVVMReplicatedTagComponent::HasAnyExactRuntimeTags(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(Flags);
}

bool UAVVMReplicatedTagComponent::HasAllExactRuntimeTags(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(Flags);
}

UAVVMReplicatedTagComponent* UAVVMReplicatedTagComponent::Static_GetActorComponent(const AActor* NewTarget)
{
	return IsValid(NewTarget) ? NewTarget->GetComponentByClass<UAVVMReplicatedTagComponent>() : nullptr;
}

void UAVVMReplicatedTagComponent::OnRep_FlagsModified(const FGameplayTagContainer OldFlags)
{
	OnReplicatedTagChanged.Broadcast(Flags);
}
