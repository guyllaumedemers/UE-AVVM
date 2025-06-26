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

#include "Abilities/GameplayAbilityTypes.h"

#include "AVVMGameplayAbilityActorInfo.generated.h"

/**
 *	Class Description :
 *
 *	FAVVMGameplayAbilityActorInfo is a scoped struct that convert the ActorInfo internal data provided by
 *	the Ability System Component to a type that manage Actor type supporting Net Connection ownership.
 *
 *	By doing so, it allows the ASC to exist on replicated actor (ex: APlayerState) that are available on both
 *	the Server and Client which isn't the case for the APlayerController. 
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	FAVVMGameplayAbilityActorInfo() = default;
	FAVVMGameplayAbilityActorInfo(const FGameplayAbilityActorInfo& NewAbilityActorInfo);
};
