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

#include "AVVMWorldSetting.h"

#include "TeamRule.generated.h"

class UPlayerStateTeamComponent;
class UTeamObject;

/**
 *	Class description:
 *
 *	UTeamBalancingCondition is a unique condition that define requirements for re-balancing a team.
 */
UCLASS(BlueprintType, Blueprintable)
class TEAMSAMPLE_API UTeamBalancingCondition : public UObject
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	UTeamBalancingImpl is a system that oversee Team balancing at Runtime.
 */
UCLASS(BlueprintType, Blueprintable)
class TEAMSAMPLE_API UTeamBalancingImpl : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSoftClassPtr<UTeamBalancingCondition>> BalancingConditionClasses;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UTeamBalancingCondition>> BalancingConditions;
};

/**
*	Class description:
 *
 *	UTeamRule is a Rule referenced in AVVMWorldSettings. This system defines requirements for building Teams.
 */
UCLASS()
class TEAMSAMPLE_API UTeamRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
	void HandleTeamAssignment(UPlayerStateTeamComponent* Player, TMap<FGameplayTag, TObjectPtr<UTeamObject>>& OutTeams) const;
	void HandleTeamRemoval(UPlayerStateTeamComponent* Player, TMap<FGameplayTag, TObjectPtr<UTeamObject>>& OutTeams) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesAllowForfaiting = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditConditionHides="bDoesAllowForfaiting", EditCondition="!bDoesForfaitingRequireAllVotes"))
	bool bDoesForfaitingRequireMajorityVotes = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditConditionHides="bDoesAllowForfaiting", EditCondition="!bDoesForfaitingRequireMajorityVotes"))
	bool bDoesForfaitingRequireAllVotes = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesAllowSwitchingTeam = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UTeamBalancingImpl> TeamBalancingImplClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UTeamBalancingImpl> TeamBalancingImpl = nullptr;
};
