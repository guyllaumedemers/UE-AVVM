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
#include "FenceManagerSubsystem.h"

#include "ActorFenceComponent.h"
#include "AVVMGameplay.h"

bool UFenceManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (IsRunningDedicatedServer())
	{
		return false;
	}

	const UWorld* PieOrGameWorld = Cast<UWorld>(Outer);
	if (IsValid(PieOrGameWorld))
	{
		const bool bIsGameClient = PieOrGameWorld->IsGameWorld();
		return bIsGameClient;
	}
	else
	{
		return false;
	}
}

void UFenceManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogGameplay, Log, TEXT("UFenceManagerSubsystem::Initialize. Running On Client."));
}

void UFenceManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	Fences.Reset();
}

UFenceManagerSubsystem* UFenceManagerSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	if (IsValid(World))
	{
		auto* Subsystem = UWorld::GetSubsystem<UFenceManagerSubsystem>(World);
		return Subsystem;
	}
	else
	{
		return nullptr;
	}
}

void UFenceManagerSubsystem::Static_UnregisterFence(const UObject* WorldContextObject,
                                                    const UActorFenceComponent* NewActorFenceComponent)
{
	if (!IsValid(WorldContextObject))
	{
		return;
	}

	auto* FenceSubsystem = UFenceManagerSubsystem::Get(WorldContextObject);
	if (IsValid(FenceSubsystem))
	{
		FenceSubsystem->Lower(NewActorFenceComponent);
	}
}

void UFenceManagerSubsystem::Static_RegisterFence(const UObject* WorldContextObject,
                                                  const UActorFenceComponent* NewActorFenceComponent)
{
	if (!IsValid(WorldContextObject))
	{
		return;
	}

	auto* FenceSubsystem = UFenceManagerSubsystem::Get(WorldContextObject);
	if (IsValid(FenceSubsystem))
	{
		FenceSubsystem->Raise(NewActorFenceComponent);
	}
}

void UFenceManagerSubsystem::Raise(const UActorFenceComponent* NewActorFenceComponent)
{
	Fences.Add(NewActorFenceComponent);
}

void UFenceManagerSubsystem::Lower(const UActorFenceComponent* NewActorFenceComponent)
{
	Fences.Remove(NewActorFenceComponent);
	if (Fences.IsEmpty())
	{
		OnAllFenceRemoved.Broadcast();
	}
}
