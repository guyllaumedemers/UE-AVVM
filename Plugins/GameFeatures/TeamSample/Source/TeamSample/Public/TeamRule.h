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

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "TeamRule.generated.h"

class UPlayerStateTeamComponent;
class UTeamObject;

/**
 *	Class description:
 *
 *	UTeamRule is a Rule referenced in AVVMWorldSettings. This system defines actions that are allowed
 *	to be executed by a member of a team.
 */
UCLASS()
class TEAMSAMPLE_API UTeamRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
	UFUNCTION(BlueprintCallable)
	const TArray<FGameplayTag>& GetTags() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesAllowForfaiting = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditConditionHides="bDoesAllowForfaiting", EditCondition="!bDoesForfaitingRequireAllVotes"))
	bool bDoesForfaitingRequireMajorityVotes = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditConditionHides="bDoesAllowForfaiting", EditCondition="!bDoesForfaitingRequireMajorityVotes"))
	bool bDoesForfaitingRequireAllVotes = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoesAllowSwitchingTeam = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="Collection of tags to create teams from."))
	TArray<FGameplayTag> TeamTags;
};
