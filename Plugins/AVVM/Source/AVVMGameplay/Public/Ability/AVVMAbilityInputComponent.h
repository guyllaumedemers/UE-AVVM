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
#pragma once

#include "CoreMinimal.h"

#include "EnhancedInputComponent.h"
#include "GameplayTags.h"
#include "Components/ActorComponent.h"

#include "AVVMAbilityInputComponent.generated.h"

class APlayerController;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;
class ULocalPlayer;

/**
 *	Class description:
 *
 *	UAVVMAbilityInputComponent setup Input Mapping Context and Input action bindings during Pawn change events. It triggers Abilities
 *	via AbilityTriggerEvents when adding/removing tags specific to the Input Action recorded.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMAbilityInputComponent : public UActorComponent
{
	GENERATED_BODY()

	struct FAVVMInputActionCallbackContext
	{
		explicit FAVVMInputActionCallbackContext(const UInputAction* NewInputAction,
		                                         const ETriggerEvent NewTriggerEvent): InputAction(NewInputAction)
		                                                                             , TriggerEvent(NewTriggerEvent)
		{
		}

		const TWeakObjectPtr<const UInputAction> InputAction = nullptr;
		const ETriggerEvent TriggerEvent = ETriggerEvent::None;
	};

public:
	UAVVMAbilityInputComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION()
	void OnPawnChanged(APawn* NewPawn,
	                   APawn* OldPawn);

	void SwapInputMappingContext(const ULocalPlayer* LocalPlayer,
	                             const APawn* NewPawn,
	                             const APawn* OldPawn) const;

	void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent, const UInputMappingContext* InputMappingContext);
	void OnInputActionReceived(const FAVVMInputActionCallbackContext InputActionCallbackContext);

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const UInputAction>, FGameplayTag> AbilityTriggerTags;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<APlayerController> OwningOuter = nullptr;
};
