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
#include "AutomatedTestSkillTreeValidationManager.h"

#include "ActorSkillTreeComponent.h"
#include "AVVMCharacter.h"
#include "AVVMPlayerState.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Resources/AVVMResourceProvider.h"

bool UAutomatedTestSkillTreeValidationManager::ShouldCreateSubsystem(UObject* Outer) const
{
#if WITH_AUTOMATION_TESTS
	const auto* World = Cast<UWorld>(Outer);
	return IsValid(World) ? World->IsGameWorld() : false;
#else
	return false;
#endif
}

void UAutomatedTestSkillTreeValidationManager::Static_RegisterComponent(const UWorld* World,
                                                                        const UActorSkillTreeComponent* SrcComponent)
{
	auto* Subsystem = UAutomatedTestSkillTreeValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->RegisterComponent(SrcComponent);
	}
}

void UAutomatedTestSkillTreeValidationManager::Static_UnregisterComponent(const UWorld* World,
                                                                          const UActorSkillTreeComponent* SrcComponent)
{
	auto* Subsystem = UAutomatedTestSkillTreeValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->UnregisterComponent(SrcComponent);
	}
}

void UAutomatedTestSkillTreeValidationManager::Static_PushPrivateItemId(const UWorld* World,
                                                                        const UActorSkillTreeComponent* SrcComponent,
                                                                        const int32 NewPrivateItemId)
{
	auto* Subsystem = UAutomatedTestSkillTreeValidationManager::Get(World);
	if (IsValid(Subsystem))
	{
		Subsystem->PushPrivateItemId(SrcComponent, NewPrivateItemId);
	}
}

UAutomatedTestSkillTreeValidationManager* UAutomatedTestSkillTreeValidationManager::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAutomatedTestSkillTreeValidationManager>(World);
}

void UAutomatedTestSkillTreeValidationManager::RegisterComponent(const UActorSkillTreeComponent* SrcComponent)
{
	if (!IsValid(SrcComponent))
	{
		return;
	}

	int32 ResourceProviderId = INDEX_NONE;

	// @gdemers characters are most-likely players, which means that we are using their UniqueNetId to identify them.
	// the execution flow require deferral so to be initialized correctly!
	const auto* Character = Cast<AAVVMCharacter>(SrcComponent->GetOuter());
	if (IsValid(Character))
	{
		auto* PlayerState = Character->GetPlayerState<AAVVMPlayerState>();
		if (IsValid(PlayerState))
		{
			PlayerState->GetOnSetPlayerUniqueNetIdDelegate().AddUObject(this, &UAutomatedTestSkillTreeValidationManager::OnPlayerStateUniqueNetIdReady, SrcComponent);
			OnPlayerStateUniqueNetIdReady(PlayerState->GetUniqueId(), SrcComponent);
		}
	}
	else
	{
		const auto* Outer = SrcComponent->GetOuter();
		ResourceProviderId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	}

	FSkillTreeValidationMechanism& OutResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutResult.ResourceProviderId = ResourceProviderId;
}

void UAutomatedTestSkillTreeValidationManager::UnregisterComponent(const UActorSkillTreeComponent* SrcComponent)
{
	const auto* SearchResult = ValidationMechanisms.Find(SrcComponent);
	if (SearchResult != nullptr)
	{
		ValidationMechanisms.Remove(SrcComponent);
	}
}

void UAutomatedTestSkillTreeValidationManager::PushPrivateItemId(const UActorSkillTreeComponent* SrcComponent,
                                                                 const int32 NewPrivateItemId)
{
	FSkillTreeValidationMechanism& OutResult = ValidationMechanisms.FindOrAdd(SrcComponent);
	OutResult.PrivateItemIds.Add(NewPrivateItemId);
}

void UAutomatedTestSkillTreeValidationManager::OnPlayerStateUniqueNetIdReady(const FUniqueNetIdRepl& UniqueNetIdRepl,
                                                                             const UActorSkillTreeComponent* SrcComponent)
{
	if (IsValid(SrcComponent) && UniqueNetIdRepl.IsValid())
	{
		const UObject* Outer = SrcComponent->GetOuter();
		FSkillTreeValidationMechanism& OutResult = ValidationMechanisms.FindOrAdd(SrcComponent);
		OutResult.ResourceProviderId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	}
}
