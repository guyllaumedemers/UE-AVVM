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
#include "AVVMCharacter.h"

#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "GameFramework/PlayerState.h"
#include "Resources/AVVMResourceManagerComponent.h"

AAVVMCharacter::AAVVMCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));

	bReplicates = true;
}

void AAVVMCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAVVMCharacter::BeginPlay()
{
	Super::BeginPlay();

	OwningActor = GetPlayerState();
}

void AAVVMCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OwningActor.Reset();
}

UAVVMAbilitySystemComponent* AAVVMCharacter::BP_GetAbilitySystemComponent() const
{
	return Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* AAVVMCharacter::GetAbilitySystemComponent() const
{
	// @gdemers Note : Override for AI derived class and provide ASC directly.
	return UAVVMAbilityUtils::GetAbilitySystemComponent(OwningActor.Get());
}

TArray<FDataRegistryId> AAVVMCharacter::GetResourceDefinitionResourceIds_Implementation() const
{
	return {ActorDefinitionId};
}

UAVVMResourceManagerComponent* AAVVMCharacter::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}
