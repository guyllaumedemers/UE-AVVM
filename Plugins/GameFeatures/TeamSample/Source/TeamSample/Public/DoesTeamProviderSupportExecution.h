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
#include "GameplayTagContainer.h"

#include "UObject/Interface.h"

#include "DoesTeamProviderSupportExecution.generated.h"

class APlayerState;

/**
 *	Class description:
 *
 *	FSwitchTeamContext is a context struct that encapsulate arguments for switching teams.
 */
USTRUCT(BlueprintType)
struct FSwitchTeamContext
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag OldTeamTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag NewTeamTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const APlayerState> PlayerState = nullptr;
};

/**
 *	Class description:
 *
 *	IDoesTeamProviderSupportExecution is an interface to be implemented by GameState to support project behaviour for below actions.
 */
UINTERFACE(BlueprintType)
class TEAMSAMPLE_API UDoesTeamProviderSupportExecution : public UInterface
{
	GENERATED_BODY()
};

class TEAMSAMPLE_API IDoesTeamProviderSupportExecution
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, CallInEditor)
	void Forfait(const APlayerState* NewPlayerState);
	virtual void Forfait_Implementation(const APlayerState* NewPlayerState) PURE_VIRTUAL(Forfait_Implementation, return;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, CallInEditor)
	void SwitchTeam(const FSwitchTeamContext& SwitchTeamContext);
	virtual void SwitchTeam_Implementation(const FSwitchTeamContext& SwitchTeamContext) PURE_VIRTUAL(SwitchTeam_Implementation, return;);
};
