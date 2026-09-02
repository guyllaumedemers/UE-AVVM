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
#include "InteractionManagerSubsystem.h"

#include "ActorInteractionComponent.h"
#include "InteractionSettings.h"
#include "Engine/World.h"

TSubclassOf<AAVVMBeaconClusterActor> FInteractionClusterSystem::GetBeaconActorClass() const
{
	return UInteractionSettings::GetBeaconClusterActorClass();
}

bool UInteractionManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	const bool bIsGameWorld = IsValid(World) ? World->IsGameWorld() : false;
	if (bIsGameWorld)
	{
		return World->GetNetMode() < NM_Client;
	}

	return false;
}

FAVVMClusterObjectHandle UInteractionManagerSubsystem::Static_Register(const UWorld* World,
                                                                       const UActorInteractionComponent* InteractionObject)
{
	auto* Subsystem = UInteractionManagerSubsystem::Get(World);
	return IsValid(Subsystem) ? Subsystem->Register(InteractionObject) : FAVVMClusterObjectHandle{};
}

bool UInteractionManagerSubsystem::Static_Unregister(const UWorld* World,
                                                     const UActorInteractionComponent* InteractionComponent,
                                                     const FAVVMClusterObjectHandle& Handle)
{
	auto* Subsystem = UInteractionManagerSubsystem::Get(World);
	return IsValid(Subsystem) ? Subsystem->Unregister(InteractionComponent, Handle) : false;
}

bool UInteractionManagerSubsystem::Static_CheckIfClosestOverlappingObject(const UWorld* World,
                                                                          const FOverlapContext& OverlapContext)
{
	auto* Subsystem = UInteractionManagerSubsystem::Get(World);
	return IsValid(Subsystem) ? Subsystem->CheckIfClosestOverlappingObject(OverlapContext) : false;
}

UInteractionManagerSubsystem* UInteractionManagerSubsystem::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UInteractionManagerSubsystem>(World);
}

FAVVMClusterObjectHandle UInteractionManagerSubsystem::Register(const UActorInteractionComponent* InteractionComponent)
{
	if (!IsValid(InteractionComponent))
	{
		return FAVVMClusterObjectHandle::InvalidHandle;
	}

	const FAVVMClusterObjectHandle OutHandle = ClusterSystem.PushPartition(GetWorld(), InteractionComponent->GetTypedOuter<AActor>());
	return OutHandle;
}

bool UInteractionManagerSubsystem::Unregister(const UActorInteractionComponent* InteractionComponent,
                                              const FAVVMClusterObjectHandle& Handle)
{
	if (!IsValid(InteractionComponent))
	{
		return false;
	}
	
	const bool bResult = ClusterSystem.PopPartition(InteractionComponent->GetTypedOuter<AActor>(), Handle);
	return bResult;
}

bool UInteractionManagerSubsystem::CheckIfClosestOverlappingObject(const FOverlapContext& OverlapContext) const
{
	const AActor* SearchResult = ClusterSystem.GetClosestOverlappingObject(OverlapContext.Handle, OverlapContext.OtherActor.Get());
	return IsValid(SearchResult) && (SearchResult == OverlapContext.Instigator);
}
