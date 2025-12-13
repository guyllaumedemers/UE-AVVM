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
#include "AVVMPlayerController.h"

#include "AVVMReplicatedTagComponent.h"
#include "Ability/AVVMAbilityInputComponent.h"
#include "Inputs/AVVMGameFrameworkInputMappingContextManager.h"

AAVVMPlayerController::AAVVMPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityInputComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAbilityInputComponent>(this, TEXT("AbilityInputComponent"));
	ReplicatedTagComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMReplicatedTagComponent>(this, TEXT("ReplicatedTagComponent"));
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickBatching = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	SetReplicateMovement(true);
	bReplicates = true;
}

void AAVVMPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AAVVMPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UAVVMGameFrameworkInputMappingContextManager::RemoveGameFrameworkInputMappingContextReceiver(this);
	
	Super::EndPlay(EndPlayReason);
}

void AAVVMPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	UAVVMGameFrameworkInputMappingContextManager::AddGameFrameworkInputMappingContextReceiver(this);
}
