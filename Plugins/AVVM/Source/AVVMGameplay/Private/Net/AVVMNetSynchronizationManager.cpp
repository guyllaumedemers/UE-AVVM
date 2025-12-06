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
#include "Net/AVVMNetSynchronizationManager.h"

#include "AVVMGameplay.h"
#include "AVVMUtils.h"
#include "Net/AVVMDoesImplNetSynchronization.h"

bool UAVVMNetSynchronizationManager::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (IsValid(World))
	{
		return World->GetNetMode() < NM_Client;
	}

	return false;
}

void UAVVMNetSynchronizationManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogGameplay, Log, TEXT("UAVVMNetSynchronizationManager::Initialize. Running On Server."));
}

void UAVVMNetSynchronizationManager::Deinitialize()
{
	Super::Deinitialize();
	NetFinalized.Reset();
}

TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>> UAVVMNetSynchronizationManager::Static_GetAllNetFinalized(const UWorld* World)
{
	const auto* NetManager = UAVVMNetSynchronizationManager::Get(World);
	return IsValid(NetManager) ? NetManager->GetAllNetFinalized() : TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>>();
}

void UAVVMNetSynchronizationManager::Static_Register(const UWorld* World, const UActorComponent* ReplicatedComponent)
{
	auto* NetManager = UAVVMNetSynchronizationManager::Get(World);
	if (IsValid(NetManager))
	{
		NetManager->Register(ReplicatedComponent);
	}
}

UAVVMNetSynchronizationManager* UAVVMNetSynchronizationManager::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAVVMNetSynchronizationManager>(World);
}

const TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>>& UAVVMNetSynchronizationManager::GetAllNetFinalized() const
{
	return NetFinalized;
}

void UAVVMNetSynchronizationManager::Register(const UActorComponent* ReplicatedComponent)
{
	const auto NetSynchronization = TScriptInterface<IAVVMDoesImplNetSynchronization>(const_cast<UActorComponent*>(ReplicatedComponent));
	const bool bIsValid = UAVVMUtils::IsNativeScriptInterfaceValid(NetSynchronization);
	if (bIsValid)
	{
		NetFinalized.AddUnique(NetSynchronization);
	}
}
