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
#include "GameplayTagContainer.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "TeamSpawnRule.generated.h"

class UAnimInstance;
class UCameraModifier;
class UNiagaraSystem;

/**
*	Class description:
 *
 *	UTeamSpawnRule is a Rule referenced in AVVMWorldSettings. This system defines behaviour specific
 *	to level spawn locations.
 *
 *	This can be used to define conditions for spawning a player in a lobby or during gameplay, and
 *	apply the modified output state during the spawning process to the ACharacter involved.
 *
 *	Example : Disabling movement in a static lobby, but still using the same ACharacter as gameplay
 *	so to use Abilities, loadout, etc...
 */
UCLASS()
class TEAMSAMPLE_API UTeamSpawnRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="True in most case, certain lobbies require static players and run simple animation."))
	bool bShouldEnableMovement = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldOverridePlayerAnimation = false;

	// @gdemers to play an ability during the spawn process on top of your output pose, as you would during character selection,
	// we suggest using AnimNotify embedded in the anim sequences.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bShouldOverridePlayerAnimation"))
	TSoftClassPtr<UAnimInstance> AnimationOverride = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldAttachVfxOnSpawn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="To spawn vfx at anchor on a Skeleton Socket or other source type.", EditCondition="bShouldAttachVfxOnSpawn"))
	TMap<FName/*Socket FName*/, TSoftClassPtr<UNiagaraSystem>> VfxOnSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldPlayAudioCueOnSpawn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bShouldPlayAudioCueOnSpawn"))
	TMap<FGameplayTag/*Src type*/, FGameplayTag/*Audio cue tag*/> AudioOnSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldApplyCameraModifier = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bShouldApplyCameraModifier"))
	TSoftClassPtr<UCameraModifier> SpawnCameraModifier = nullptr;
};
