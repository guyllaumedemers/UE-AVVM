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
#include "Ability/AVVMAbilityInputComponent.h"

#include "AbilitySystemInterface.h"
#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Inputs/AVVMAbilityInputAction.h"
#include "Inputs/AVVMInputMappingProvider.h"

void UAVVMAbilityInputComponent::BeginPlay()
{
	Super::BeginPlay();

	auto* PlayerState = GetTypedOuter<APlayerState>();
	if (IsValid(PlayerState) && UAVVMGameplayUtils::IsLocallyControlled(PlayerState))
	{
		PlayerState->OnPawnSet.AddUniqueDynamic(this, &UAVVMAbilityInputComponent::OnPawnChanged);
		OnPawnChanged(PlayerState, PlayerState->GetPawn(), nullptr);
	}

	OwningOuter = PlayerState;
}

void UAVVMAbilityInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto* PlayerState = OwningOuter.Get();
	if (IsValid(PlayerState) && UAVVMGameplayUtils::IsLocallyControlled(PlayerState))
	{
		PlayerState->OnPawnSet.RemoveAll(this);
	}

	OwningOuter.Reset();
	AbilityTriggerTags.Reset();
}

void UAVVMAbilityInputComponent::OnPawnChanged(APlayerState* Player,
                                               APawn* NewPawn,
                                               APawn* OldPawn)
{
	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IAVVMInputMappingProvider, UAVVMInputMappingProvider>(NewPawn);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IAVVMInputMappingProvider interface!")))
	{
		return;
	}

	const auto* PlayerController = Cast<APlayerController>(NewPawn->GetController());
	if (IsValid(PlayerController))
	{
		const ULocalPlayer* LocalPLayer = PlayerController->GetLocalPlayer();
		UEnhancedInputComponent* InputComponent = NewPawn->GetComponentByClass<UEnhancedInputComponent>();

		SwapInputMappingContext(LocalPLayer, NewPawn, OldPawn);
		BindInputActions(InputComponent, IAVVMInputMappingProvider::Execute_GetInputMappingContext(NewPawn));
	}
}

void UAVVMAbilityInputComponent::SwapInputMappingContext(const ULocalPlayer* LocalPlayer,
                                                         const APawn* NewPawn,
                                                         const APawn* OldPawn) const
{
	auto* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!IsValid(EnhancedInputSubsystem))
	{
		return;
	}

	const APlayerState* PlayerState = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(PlayerState), TEXT("Invalid Actor!")))
	{
		return;
	}

	const UInputMappingContext* OldInputMappingContext = IAVVMInputMappingProvider::Execute_GetInputMappingContext(OldPawn);
	if (IsValid(OldInputMappingContext))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Removing Input Mapping Context \"%s\" on Actor \"%s\"."),
		       *OldInputMappingContext->GetName(),
		       *PlayerState->GetName());

		EnhancedInputSubsystem->RemoveMappingContext(OldInputMappingContext);
	}

	const UInputMappingContext* NewInputMappingContext = IAVVMInputMappingProvider::Execute_GetInputMappingContext(NewPawn);
	if (IsValid(NewInputMappingContext))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Adding Input Mapping Context \"%s\" on Actor \"%s\"."),
		       *NewInputMappingContext->GetName(),
		       *PlayerState->GetName());

		EnhancedInputSubsystem->AddMappingContext(NewInputMappingContext, 0);
	}
}

void UAVVMAbilityInputComponent::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent, const UInputMappingContext* InputMappingContext)
{
	if (!IsValid(EnhancedInputComponent) || !IsValid(InputMappingContext))
	{
		return;
	}

	const APlayerState* PlayerState = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(PlayerState), TEXT("Invalid Actor!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Clearing Input Action Bindings on Actor \"%s\"."),
	       *PlayerState->GetName());

	EnhancedInputComponent->ClearActionBindings();
	AbilityTriggerTags.Reset();

	for (const FEnhancedActionKeyMapping& ActionKeyMapping : InputMappingContext->GetMappings())
	{
		const auto* InputAction = Cast<UAVVMAbilityInputAction>(ActionKeyMapping.Action);
		if (!IsValid(InputAction))
		{
			continue;
		}

		FGameplayTag& OutTag = AbilityTriggerTags.FindOrAdd(InputAction);
		OutTag = InputAction->AbilityTriggerTag;

		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Registering New Input Action \"%s\" and Tag \"%s\" on Actor \"%s\"."),
		       *InputAction->GetName(),
		       *OutTag.ToString(),
		       *PlayerState->GetName());

		EnhancedInputComponent->BindAction(InputAction,
		                                   ETriggerEvent::Started,
		                                   this,
		                                   &UAVVMAbilityInputComponent::OnInputActionReceived,
		                                   FAVVMInputActionCallbackContext{InputAction, ETriggerEvent::Started});

		EnhancedInputComponent->BindAction(InputAction,
		                                   ETriggerEvent::Canceled,
		                                   this,
		                                   &UAVVMAbilityInputComponent::OnInputActionReceived,
		                                   FAVVMInputActionCallbackContext{InputAction, ETriggerEvent::Canceled});

		EnhancedInputComponent->BindAction(InputAction,
		                                   ETriggerEvent::Completed,
		                                   this,
		                                   &UAVVMAbilityInputComponent::OnInputActionReceived,
		                                   FAVVMInputActionCallbackContext{InputAction, ETriggerEvent::Completed});
	}
}

void UAVVMAbilityInputComponent::OnInputActionReceived(const FAVVMInputActionCallbackContext InputActionCallbackContext)
{
	const auto AbilityInterface = TScriptInterface<IAbilitySystemInterface>(OwningOuter.Get());
	if (!UAVVMUtilityFunctionLibrary::IsNativeScriptInterfaceValid(AbilityInterface))
	{
		return;
	}

	auto* AbilitySystemComponent = Cast<UAVVMAbilitySystemComponent>(AbilityInterface->GetAbilitySystemComponent());
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	const FGameplayTag* SearchTag = AbilityTriggerTags.Find(InputActionCallbackContext.InputAction);
	if (!ensureAlwaysMsgf(SearchTag != nullptr, TEXT("Input Action doesn't match to any registered entries.")))
	{
		return;
	}

	const bool bCanRemoveTag = EnumHasAnyFlags(InputActionCallbackContext.TriggerEvent, ETriggerEvent::Canceled | ETriggerEvent::Completed);
	if (bCanRemoveTag)
	{
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(*SearchTag);
		return;
	}

	const bool bCanAddTag = EnumHasAnyFlags(InputActionCallbackContext.TriggerEvent, ETriggerEvent::Started);
	if (bCanAddTag)
	{
		AbilitySystemComponent->AddReplicatedLooseGameplayTag(*SearchTag);
		return;
	}
}
