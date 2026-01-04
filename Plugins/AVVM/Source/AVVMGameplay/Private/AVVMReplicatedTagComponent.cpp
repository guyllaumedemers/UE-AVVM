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

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
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

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" Class Instance to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAVVMReplicatedTagComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter = Outer;

	const FGameplayTagContainer CachePendingTags = PendingFlags;
	PendingFlags.Reset();
	Append(CachePendingTags);
}

void UAVVMReplicatedTagComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" Class Instance to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAVVMReplicatedTagComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

void UAVVMReplicatedTagComponent::Append(const FGameplayTagContainer& NewTags)
{
	if (!OwningOuter.IsValid())
	{
		// @gdemers  There is cases in which we try appending as soon as the local player gets assigned a PC so to be able
		// to track adding primary game layout to the viewport resulting in the fence system lowering a condition.
		// The problem however is that Actor construction of the PC which initialize the component above only happens later after receiving the local player.
		PendingFlags.AppendTags(NewTags);
		return;
	}

	const FGameplayTagContainer OldTags = Flags;
	Flags.AppendTags(NewTags);
	MARK_PROPERTY_DIRTY_FROM_NAME(UAVVMReplicatedTagComponent, Flags, this);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding Tag \"%s\". Active Tags \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData(),
	       *NewTags.ToString(),
	       *Flags.ToString())

	OnRep_FlagsModified(OldTags);
}

void UAVVMReplicatedTagComponent::Remove(const FGameplayTagContainer& NewTags)
{
	const FGameplayTagContainer OldTags = Flags;
	Flags.RemoveTags(NewTags);
	MARK_PROPERTY_DIRTY_FROM_NAME(UAVVMReplicatedTagComponent, Flags, this);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing Tag \"%s\". Active Tags \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(OwningOuter.Get()).GetData(),
	       *NewTags.ToString(),
	       *Flags.ToString())

	OnRep_FlagsModified(OldTags);
}

bool UAVVMReplicatedTagComponent::HasAnyExact(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAnyExact(Flags);
}

bool UAVVMReplicatedTagComponent::HasAllExact(const FGameplayTagContainer& Compare) const
{
	return Compare.HasAllExact(Flags);
}

UAVVMReplicatedTagComponent* UAVVMReplicatedTagComponent::GetActorComponent(const AActor* NewTarget)
{
	return IsValid(NewTarget) ? NewTarget->GetComponentByClass<UAVVMReplicatedTagComponent>() : nullptr;
}

void UAVVMReplicatedTagComponent::OnRep_FlagsModified(const FGameplayTagContainer OldFlags)
{
	OnReplicatedTagChanged.Broadcast(Flags);
}
