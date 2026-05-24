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
#include "ActorSkillTreeComponent.h"

#include "AVVMLogger.h"
#include "SkillSampleModule.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UActorSkillTreeComponent_InstanceCounter, TEXT("SkillTree Component Instance Counter"));

UActorSkillTreeComponent::UActorSkillTreeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = true;
}

void UActorSkillTreeComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
}

void UActorSkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UActorSkillTreeComponent_InstanceCounter);
	AVVM_LOGGER_LOG(LogSkillSample,
					Outer,
					Outer,
					TEXT("Adding %s."),
					*GetNameSafe(UActorSkillTreeComponent::StaticClass()));

	OwningOuter = Outer;
}

void UActorSkillTreeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogSkillSample,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UActorSkillTreeComponent::StaticClass()));

	OwningOuter.Reset();
}

UActorSkillTreeComponent* UActorSkillTreeComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UActorSkillTreeComponent>() : nullptr;
}
