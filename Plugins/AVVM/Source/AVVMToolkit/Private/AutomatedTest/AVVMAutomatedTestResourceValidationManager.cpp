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
#include "AutomatedTest/AVVMAutomatedTestResourceValidationManager.h"

#include "Engine/World.h"
#include "Misc/AutomationTest.h"

FAVVMNonTSResourceValidationMechanism::FAVVMNonTSResourceValidationMechanism()
	: RegistryIdRequested(0)
	, RegistryIdLoaded(0)
	, UObjectRequested(0)
	, UObjectLoaded(0)
{
}

bool UAVVMAutomatedTestResourceValidationManager::ShouldCreateSubsystem(UObject* Outer) const
{
#if WITH_AUTOMATION_TESTS
	const auto* World = Cast<UWorld>(Outer);
	return IsValid(World) ? World->IsGameWorld() : false;
#else
	return false;
#endif
}

void UAVVMAutomatedTestResourceValidationManager::Static_RegisterComponent(const UWorld* World, const UActorComponent* SrcComponent)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->RegisterComponent(SrcComponent);
	}
}

void UAVVMAutomatedTestResourceValidationManager::Static_UnregisterComponent(const UWorld* World, const UActorComponent* SrcComponent)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->UnregisterComponent(SrcComponent);
	}
}

void UAVVMAutomatedTestResourceValidationManager::Static_IncrementRegistryIdRequested(const UWorld* World,
                                                                                      const UActorComponent* SrcComponent,
                                                                                      const int32 NewValue)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->IncrementRegistryIdRequested(SrcComponent, NewValue);
	}
}

void UAVVMAutomatedTestResourceValidationManager::Static_IncrementRegistryIdLoaded(const UWorld* World,
                                                                                   const UActorComponent* SrcComponent,
                                                                                   const int32 NewValue)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->IncrementRegistryIdLoaded(SrcComponent, NewValue);
	}
}

void UAVVMAutomatedTestResourceValidationManager::Static_IncrementUObjectRequested(const UWorld* World,
                                                                                   const UActorComponent* SrcComponent,
                                                                                   const int32 NewValue)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->IncrementUObjectRequested(SrcComponent, NewValue);
	}
}

void UAVVMAutomatedTestResourceValidationManager::Static_IncrementUObjectLoaded(const UWorld* World,
                                                                                const UActorComponent* SrcComponent,
                                                                                const int32 NewValue)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->IncrementUObjectLoaded(SrcComponent, NewValue);
	}
}

bool UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(const UWorld* World, const UActorComponent* SrcComponent)
{
	auto* Subsystem = UAVVMAutomatedTestResourceValidationManager::Get(World);
	return IsValid(Subsystem)
			? Subsystem->IsIntegral(SrcComponent)
			: false;
}

UAVVMAutomatedTestResourceValidationManager* UAVVMAutomatedTestResourceValidationManager::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAVVMAutomatedTestResourceValidationManager>(World);
}

void UAVVMAutomatedTestResourceValidationManager::RegisterComponent(const UActorComponent* SrcComponent)
{
	ValidationMechanisms.FindOrAdd(SrcComponent);
}

void UAVVMAutomatedTestResourceValidationManager::UnregisterComponent(const UActorComponent* SrcComponent)
{
	const auto* SearchResult = ValidationMechanisms.Find(SrcComponent);
	if (SearchResult != nullptr)
	{
		ValidationMechanisms.Remove(SrcComponent);
	}
}

void UAVVMAutomatedTestResourceValidationManager::IncrementRegistryIdRequested(const UActorComponent* SrcComponent, const int32 NewValue)
{
	FAVVMNonTSResourceValidationMechanism& OutSearchResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutSearchResult.RegistryIdRequested += NewValue;
}

void UAVVMAutomatedTestResourceValidationManager::IncrementRegistryIdLoaded(const UActorComponent* SrcComponent, const int32 NewValue)
{
	FAVVMNonTSResourceValidationMechanism& OutSearchResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutSearchResult.RegistryIdLoaded += NewValue;
}

void UAVVMAutomatedTestResourceValidationManager::IncrementUObjectRequested(const UActorComponent* SrcComponent, const int32 NewValue)
{
	FAVVMNonTSResourceValidationMechanism& OutSearchResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutSearchResult.UObjectRequested += NewValue;
}

void UAVVMAutomatedTestResourceValidationManager::IncrementUObjectLoaded(const UActorComponent* SrcComponent, const int32 NewValue)
{
	FAVVMNonTSResourceValidationMechanism& OutSearchResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutSearchResult.UObjectLoaded += NewValue;
}

bool UAVVMAutomatedTestResourceValidationManager::IsIntegral(const UActorComponent* SrcComponent)
{
	FAVVMNonTSResourceValidationMechanism& OutSearchResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	return (OutSearchResult.RegistryIdRequested == OutSearchResult.RegistryIdLoaded)
			&& (OutSearchResult.UObjectRequested == OutSearchResult.UObjectLoaded);
}
