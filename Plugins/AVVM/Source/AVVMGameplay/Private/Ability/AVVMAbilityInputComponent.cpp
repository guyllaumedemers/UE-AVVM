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

#include "AbilitySystemBlueprintLibrary.h"
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

	auto* Outer = GetTypedOuter<APlayerState>();
	if (IsValid(Outer) && UAVVMGameplayUtils::IsLocallyControlled(Outer))
	{
		OwningOuter = Outer;
		Outer->OnPawnSet.AddUniqueDynamic(this, &UAVVMAbilityInputComponent::OnPawnChanged);
		OnPawnChanged(Outer, Outer->GetPawn(), nullptr);
	}
}

void UAVVMAbilityInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto* Outer = OwningOuter.Get();
	if (IsValid(Outer) && UAVVMGameplayUtils::IsLocallyControlled(Outer))
	{
		Outer->OnPawnSet.RemoveAll(this);
	}

	OwningOuter.Reset();
	AbilityTriggerTags.Reset();
}

void UAVVMAbilityInputComponent::OnPawnChanged(APlayerState* Player,
                                               APawn* NewPawn,
                                               APawn* OldPawn)
{
	if (!IsValid(NewPawn))
	{
		return;
	}

	const bool bResult = UAVVMUtilityFunctionLibrary::DoesImplementNativeOrBlueprintInterface<IAVVMInputMappingProvider, UAVVMInputMappingProvider>(NewPawn);
	if (!ensureAlwaysMsgf(bResult, TEXT("Outer doesn't implement the IAVVMInputMappingProvider interface!")))
	{
		return;
	}

	const auto* PlayerController = Cast<APlayerController>(NewPawn->GetController());
	if (IsValid(PlayerController))
	{
		auto* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		const ULocalPlayer* LocalPLayer = PlayerController->GetLocalPlayer();

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

	const APlayerState* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const UInputMappingContext* OldInputMappingContext = IsValid(OldPawn) ? IAVVMInputMappingProvider::Execute_GetInputMappingContext(OldPawn) : nullptr;
	if (IsValid(OldInputMappingContext))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Removing Input Mapping Context \"%s\" on Outer \"%s\"."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
		       *OldInputMappingContext->GetName(),
		       *Outer->GetName());

		EnhancedInputSubsystem->RemoveMappingContext(OldInputMappingContext);
	}

	const UInputMappingContext* NewInputMappingContext = IsValid(NewPawn) ? IAVVMInputMappingProvider::Execute_GetInputMappingContext(NewPawn) : nullptr;
	if (IsValid(NewInputMappingContext))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Adding Input Mapping Context \"%s\" on Outer \"%s\"."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
		       *NewInputMappingContext->GetName(),
		       *Outer->GetName());

		EnhancedInputSubsystem->AddMappingContext(NewInputMappingContext, 0);
	}
}

void UAVVMAbilityInputComponent::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent, const UInputMappingContext* InputMappingContext)
{
	if (!IsValid(EnhancedInputComponent) || !IsValid(InputMappingContext))
	{
		return;
	}

	const APlayerState* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Clearing Input Action Bindings on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
	       *Outer->GetName());

	EnhancedInputComponent->ClearBindingsForObject(this);
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
		       TEXT("Executed from \"%s\". Registering New Input Action \"%s\" and Tag \"%s\" on Outer \"%s\"."),
		       UAVVMGameplayUtils::PrintNetMode(Outer).GetData(),
		       *InputAction->GetName(),
		       *OutTag.ToString(),
		       *Outer->GetName());

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
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningOuter.Get());
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
		AbilitySystemComponent->RemoveLooseGameplayTag(*SearchTag);
		return;
	}

	const bool bCanAddTag = EnumHasAnyFlags(InputActionCallbackContext.TriggerEvent, ETriggerEvent::Started);
	if (bCanAddTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(*SearchTag);
		return;
	}
}
