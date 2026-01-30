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

#include "DataRegistryId.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"

#include "AVVMGameplaySettings.generated.h"

/**
 *	Class description:
 *
 *	UAVVMGameplaySettings is a DeveloperSettings that expose global data specific to gameplay.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UAVVMGameplaySettings"))
class AVVMGAMEPLAY_API UAVVMGameplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AVVMGameplay|Settings")
	static const FDataRegistryType& GetActorIdentifierRegistryType();
	
	UFUNCTION(BlueprintCallable, Category="AVVMGameplay|Settings")
	static const FGameplayTagContainer& GetBlockingTags_PlayerAction();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FDataRegistryType ActorIdentifierRegistryType = FDataRegistryType();

	// @gdemers defined tags that are able to prevent user from executing actions based on conditions. (i.e Stasis, Down, etc...)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTagContainer BlockingTags_PlayerAction = FGameplayTagContainer::EmptyContainer;
};
