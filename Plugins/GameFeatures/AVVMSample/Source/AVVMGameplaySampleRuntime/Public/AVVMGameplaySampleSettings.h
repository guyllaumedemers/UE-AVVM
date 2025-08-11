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

#include "Components/ActorComponent.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"

#include "AVVMGameplaySampleSettings.generated.h"

/**
*	Class description:
 *
 *	UAVVMGameplaySampleSettings is a UDeveloperSettings that define a set of Component Class, specific to your project,
 *	and from which we run cheat against.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UAVVMGameplaySampleSettings"))
class AVVMGAMEPLAYSAMPLERUNTIME_API UAVVMGameplaySampleSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UActorComponent> GetHealthComponentClass();

	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UActorComponent> GetStaminaComponentClass();

	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UActorComponent> GetStatusEffectComponentClass();

	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UActorComponent> GetAbilityComponentClass();

	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UActorComponent> GetCurrencyComponentClass();

	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UActorComponent> GetInteractionComponentClass();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UActorComponent> HealthComponentClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UActorComponent> StaminaComponentClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UActorComponent> StatusEffectComponentClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UActorComponent> AbilityComponentClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UActorComponent> CurrencyComponentClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UActorComponent> InteractionComponentClass = nullptr;
};
