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
#include "AutomatedTestInventoryValidationManager.h"

#include "ActorInventoryComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Resources/AVVMResourceProvider.h"

bool UAutomatedTestInventoryValidationManager::ShouldCreateSubsystem(UObject* Outer) const
{
#if WITH_AUTOMATION_TESTS
	const auto* World = Cast<UWorld>(Outer);
	return IsValid(World) ? World->IsGameWorld() : false;
#else
	return false;
#endif
}

void UAutomatedTestInventoryValidationManager::Static_RegisterComponent(const UWorld* World,
                                                                        const UActorInventoryComponent* SrcComponent)
{
	auto* Subsystem = UAutomatedTestInventoryValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->RegisterComponent(SrcComponent);
	}
}

void UAutomatedTestInventoryValidationManager::Static_UnregisterComponent(const UWorld* World,
                                                                          const UActorInventoryComponent* SrcComponent)
{
	auto* Subsystem = UAutomatedTestInventoryValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->UnregisterComponent(SrcComponent);
	}
}

void UAutomatedTestInventoryValidationManager::Static_PushPrivateItemId(const UWorld* World,
                                                                        const UActorInventoryComponent* SrcComponent,
                                                                        const int32 NewPrivateItemId)
{
	auto* Subsystem = UAutomatedTestInventoryValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->PushPrivateItemId(SrcComponent, NewPrivateItemId);
	}
}

UAutomatedTestInventoryValidationManager* UAutomatedTestInventoryValidationManager::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAutomatedTestInventoryValidationManager>(World);
}

void UAutomatedTestInventoryValidationManager::RegisterComponent(const UActorInventoryComponent* SrcComponent)
{
	if (IsValid(SrcComponent))
	{
		const UObject* Outer = SrcComponent->GetOuter();
		FInventoryValidationMechanism& OutResult = ValidationMechanisms.FindOrAdd(SrcComponent);
		OutResult.ResourceProviderId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	}
}

void UAutomatedTestInventoryValidationManager::UnregisterComponent(const UActorInventoryComponent* SrcComponent)
{
	const auto* SearchResult = ValidationMechanisms.Find(SrcComponent);
	if (SearchResult != nullptr)
	{
		ValidationMechanisms.Remove(SrcComponent);
	}
}

void UAutomatedTestInventoryValidationManager::PushPrivateItemId(const UActorInventoryComponent* SrcComponent,
                                                                 const int32 NewPrivateItemId)
{
	FInventoryValidationMechanism& OutResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutResult.PrivateItemIds.Add(NewPrivateItemId);
}
