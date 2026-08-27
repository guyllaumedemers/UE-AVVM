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

#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"

#include "InteractionSettings.generated.h"

class AAVVMBeaconClusterActor;

/**
 *	Class description:
 *
 *	UInteractionDeveloperSettings is a DeveloperSettings that expose global data specific to gameplay.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UInteractionSettings"))
class INTERACTIONSAMPLE_API UInteractionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UInteractionSettings();
	
	UFUNCTION(BlueprintCallable, Category="InteractionSample|Settings")
	static TSubclassOf<AAVVMBeaconClusterActor> GetBeaconClusterActorClass();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<AAVVMBeaconClusterActor> BeaconClusterActorClass = nullptr;
};
