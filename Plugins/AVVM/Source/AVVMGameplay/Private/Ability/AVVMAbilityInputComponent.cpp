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
#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Inputs/AVVMAbilityInputAction.h"
#include "Inputs/AVVMInputMappingProvider.h"

UAVVMAbilityInputComponent::UAVVMAbilityInputComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(false);
}

void UAVVMAbilityInputComponent::BeginPlay()
{
	Super::BeginPlay();

	auto* Outer = GetTypedOuter<APlayerController>();
	if (IsValid(Outer) && Outer->IsLocalPlayerController())
	{
		OwningOuter = Outer;
		Outer->OnPossessedPawnChanged.AddDynamic(this, &UAVVMAbilityInputComponent::OnPawnChanged);
		OnPawnChanged(Outer->GetPawn(), nullptr);
	}
}

void UAVVMAbilityInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto* Outer = OwningOuter.Get();
	if (IsValid(Outer) && Outer->IsLocalPlayerController())
	{
		Outer->OnPossessedPawnChanged.RemoveAll(this);

		AbilityTriggerTags.Reset();
		BindingHandles.Reset();
		OwningOuter.Reset();
	}
}

void UAVVMAbilityInputComponent::OnPawnChanged(APawn* NewPawn,
                                               APawn* OldPawn)
{
	if (!IsValid(NewPawn))
	{
		return;
	}

	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMInputMappingProvider>(NewPawn);
	if (!bResult)
	{
		return;
	}

	const auto* PlayerController = Cast<APlayerController>(NewPawn->GetController());
	if (IsValid(PlayerController))
	{
		auto* InputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

		SwapInputMappingContext(LocalPlayer, NewPawn, OldPawn, InputComponent);
	}
}

void UAVVMAbilityInputComponent::SwapInputMappingContext(const ULocalPlayer* LocalPlayer,
                                                         const APawn* NewPawn,
                                                         const APawn* OldPawn,
                                                         UEnhancedInputComponent* EnhancedInputComponent)
{
	auto* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	// @gdemers remove old bindings, and imc
	const UInputMappingContext* OldInputMappingContext = IsValid(OldPawn) ? IAVVMInputMappingProvider::Execute_GetInputMappingContext(OldPawn) : nullptr;
	
	UnBindInputActions(EnhancedInputComponent, OldInputMappingContext);
	UnRegisterInputMappingContext(EnhancedInputSubsystem, OldInputMappingContext);

	// @gdemers add new bindings, and imc
	const UInputMappingContext* NewInputMappingContext = IsValid(NewPawn) ? IAVVMInputMappingProvider::Execute_GetInputMappingContext(NewPawn) : nullptr;
	
	RegisterInputMappingContext(EnhancedInputSubsystem, NewInputMappingContext);
	BindInputActions(EnhancedInputComponent, NewInputMappingContext);
}

void UAVVMAbilityInputComponent::UnRegisterInputMappingContext(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem,
                                                               const UInputMappingContext* InputMappingContext) const
{
	if (!IsValid(EnhancedInputSubsystem))
	{
		return;
	}

	const APlayerController* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (IsValid(InputMappingContext))
	{
		EnhancedInputSubsystem->RemoveMappingContext(InputMappingContext);
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Removing Input Mapping Context %s."),
	                *GetNameSafe(InputMappingContext));
}

void UAVVMAbilityInputComponent::RegisterInputMappingContext(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem,
                                                             const UInputMappingContext* InputMappingContext) const
{
	if (!IsValid(EnhancedInputSubsystem))
	{
		return;
	}

	const APlayerController* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (IsValid(InputMappingContext))
	{
		static int32 Priority = 0;
		EnhancedInputSubsystem->AddMappingContext(InputMappingContext, Priority++);
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Adding Input Mapping Context %s."),
	                *GetNameSafe(InputMappingContext));
}

void UAVVMAbilityInputComponent::UnBindInputActions(UEnhancedInputComponent* EnhancedInputComponent,
                                                    const UInputMappingContext* InputMappingContext)
{
	if (!IsValid(EnhancedInputComponent) || !IsValid(InputMappingContext))
	{
		return;
	}

	const APlayerController* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	for (const FEnhancedActionKeyMapping& ActionKeyMapping : InputMappingContext->GetMappings())
	{
		const auto* InputAction = Cast<UAVVMAbilityInputAction>(ActionKeyMapping.Action);
		if (!IsValid(InputAction))
		{
			continue;
		}

		AbilityTriggerTags.Remove(InputAction);

		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("UnRegistering Old Input Action %s and Tag %s."),
		                *GetNameSafe(InputAction),
		                *InputAction->AbilityTriggerTag.ToString());

		FAVVMEnhancedInputEventBindingHandles& OutResult = BindingHandles.FindOrAdd(InputAction);
		for (const uint32 Handle : OutResult.Handles)
		{
			EnhancedInputComponent->RemoveBindingByHandle(Handle);
		}
	}
}

void UAVVMAbilityInputComponent::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent,
                                                  const UInputMappingContext* InputMappingContext)
{
	if (!IsValid(EnhancedInputComponent) || !IsValid(InputMappingContext))
	{
		return;
	}

	const APlayerController* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	for (const FEnhancedActionKeyMapping& ActionKeyMapping : InputMappingContext->GetMappings())
	{
		const auto* InputAction = Cast<UAVVMAbilityInputAction>(ActionKeyMapping.Action);
		if (!IsValid(InputAction))
		{
			continue;
		}

		FGameplayTag& OutTag = AbilityTriggerTags.FindOrAdd(InputAction);
		OutTag = InputAction->AbilityTriggerTag;

		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("Registering New Input Action %s and Tag %s."),
		                *GetNameSafe(InputAction),
		                *OutTag.ToString());

		FEnhancedInputActionEventBinding& BindingHandle_Started = EnhancedInputComponent->BindAction(InputAction,
		                                                                                             ETriggerEvent::Started,
		                                                                                             this,
		                                                                                             &UAVVMAbilityInputComponent::OnInputActionReceived,
		                                                                                             FAVVMInputActionCallbackContext{InputAction, ETriggerEvent::Started});

		FEnhancedInputActionEventBinding& BindingHandle_Canceled = EnhancedInputComponent->BindAction(InputAction,
		                                                                                              ETriggerEvent::Canceled,
		                                                                                              this,
		                                                                                              &UAVVMAbilityInputComponent::OnInputActionReceived,
		                                                                                              FAVVMInputActionCallbackContext{InputAction, ETriggerEvent::Canceled});

		FEnhancedInputActionEventBinding& BindingHandle_Completed = EnhancedInputComponent->BindAction(InputAction,
		                                                                                               ETriggerEvent::Completed,
		                                                                                               this,
		                                                                                               &UAVVMAbilityInputComponent::OnInputActionReceived,
		                                                                                               FAVVMInputActionCallbackContext{InputAction, ETriggerEvent::Completed});
		
		FAVVMEnhancedInputEventBindingHandles& OutResult = BindingHandles.FindOrAdd(InputAction);
		OutResult.Handles.Add(BindingHandle_Started.GetHandle());
		OutResult.Handles.Add(BindingHandle_Canceled.GetHandle());
		OutResult.Handles.Add(BindingHandle_Completed.GetHandle());
	}
}

void UAVVMAbilityInputComponent::OnInputActionReceived(const FAVVMInputActionCallbackContext InputActionCallbackContext)
{
	APlayerController* PC = OwningOuter.Get();
	if (!IsValid(PC))
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PC->PlayerState);
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	const auto* InputAction = Cast<UAVVMAbilityInputAction>(InputActionCallbackContext.InputAction.Get());
	if (!ensureAlwaysMsgf(IsValid(InputAction), TEXT("Input Action doesn't derive from UAVVMAbilityInputAction.")))
	{
		return;
	}

	const FGameplayTag* SearchTag = AbilityTriggerTags.Find(InputAction);
	if (!ensureAlwaysMsgf(SearchTag != nullptr, TEXT("Input Action doesn't match to any registered entries.")))
	{
		return;
	}

	const bool bCanRemoveTag = EnumHasAnyFlags(InputActionCallbackContext.TriggerEvent, ETriggerEvent::Canceled | ETriggerEvent::Completed);
	if (bCanRemoveTag)
	{
		AbilitySystemComponent->ReleaseInputID(InputAction->GetInputId());
		AbilitySystemComponent->RemoveLooseGameplayTag(*SearchTag);
		return;
	}

	const bool bCanAddTag = EnumHasAnyFlags(InputActionCallbackContext.TriggerEvent, ETriggerEvent::Started);
	if (bCanAddTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(*SearchTag);
		AbilitySystemComponent->PressInputID(InputAction->GetInputId());
		return;
	}
}

void UAVVMAbilityInputComponent::UnRegisterGameFrameworkIMCs(const TArray<const UInputMappingContext*>& IMCs)
{
	auto* PC = Cast<APlayerController>(OwningOuter.Get());
	if (!IsValid(PC))
	{
		return;
	}

	auto* InputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!IsValid(InputComponent))
	{
		return;
	}

	auto* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	for (const UInputMappingContext* IMC : IMCs)
	{
		UnRegisterInputMappingContext(EnhancedInputSubsystem, IMC);
	}
}

void UAVVMAbilityInputComponent::RegisterGameFrameworkIMCs(const TArray<const UInputMappingContext*>& IMCs)
{
	// @gdemers often suffer from race conditions as BeginPlay was not yet called.
	if (!OwningOuter.IsValid())
	{
		OwningOuter = GetTypedOuter<APlayerController>();
	}

	auto* PC = OwningOuter.Get();
	if (!IsValid(PC))
	{
		return;
	}

	auto* InputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!IsValid(InputComponent))
	{
		return;
	}

	auto* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	for (const UInputMappingContext* IMC : IMCs)
	{
		RegisterInputMappingContext(EnhancedInputSubsystem, IMC);
		BindInputActions(InputComponent, IMC);
	}
}
