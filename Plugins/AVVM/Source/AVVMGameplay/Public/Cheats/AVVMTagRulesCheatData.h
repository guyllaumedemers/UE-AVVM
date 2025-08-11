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
#include "Cheats/AVVMCheatData.h"

#include "AVVMTagRulesCheatData.generated.h"

/**
 *	Class description:
 *
 *	FAVVMTagRulesCheatData is a POD class that encapsulate tag requirements for testing conditional visibility on the HUD. In short,
 *	a TagRuleset may define a scenario specific to the gameplay.
 *
 *	Example : Player.InTutorial & Player.HasNoWeapon & Player.HasVignetting etc...
 *
 *	Note : To be able to test using ImGui. Look into the following tag - AVVMGameplay.UIChannel.HUD.ModifyRuleset. Our HUD widget will
 *	be bound to a View Model and owned by a presenter.
 */
USTRUCT()
struct AVVMGAMEPLAY_API FAVVMTagRulesCheatData : public FAVVMCheatData
{
	GENERATED_BODY()

	const FGameplayTagContainer& GetRequirementTags() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer RequirementTags = FGameplayTagContainer::EmptyContainer;
};
