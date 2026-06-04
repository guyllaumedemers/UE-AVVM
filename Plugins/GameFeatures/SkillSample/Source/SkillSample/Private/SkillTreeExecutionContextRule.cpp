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
#include "SkillTreeExecutionContextRule.h"

#include "ActorSkillTreeComponent.h"
#include "AVVMCharacter.h"
#include "AVVMGameplaySettings.h"
#include "SkillTreeExecutionContextParams.h"

bool FGrantRule::Predicate(const UActorComponent* Component,
                           const TInstancedStruct<FExecutionContextParams>& Params) const
{
	auto* SkillTreeComponent = Cast<UActorSkillTreeComponent>(Component);
	if (!IsValid(SkillTreeComponent))
	{
		return false;
	}

	// @gdemers first, validate if our actor is in a state where they can act on a skill tree node.
	const auto* Player = SkillTreeComponent->GetTypedOuter<AAVVMCharacter>();
	if (IsValid(Player) && Player->HasPartialMatch(UAVVMGameplaySettings::GetPlayerAbilityBlockingTags()))
	{
		return false;
	}

	const auto* GrantParams = Params.GetPtr<FGrantContextParams>();
	if (!ensureAlwaysMsgf(GrantParams != nullptr,
	                      TEXT("FExecutionContextParams couldn't be cast to FGrantContextParams.")))
	{
		return false;
	}
}

bool FRevokeRule::Predicate(const UActorComponent* Component,
                            const TInstancedStruct<FExecutionContextParams>& Params) const
{
	auto* SkillTreeComponent = Cast<UActorSkillTreeComponent>(Component);
	if (!IsValid(SkillTreeComponent))
	{
		return false;
	}

	// @gdemers first, validate if our actor is in a state where they can act on a skill tree node.
	const auto* Player = SkillTreeComponent->GetTypedOuter<AAVVMCharacter>();
	if (IsValid(Player) && Player->HasPartialMatch(UAVVMGameplaySettings::GetPlayerAbilityBlockingTags()))
	{
		return false;
	}

	const auto* RevokeParams = Params.GetPtr<FRevokeContextParams>();
	if (!ensureAlwaysMsgf(RevokeParams != nullptr,
	                      TEXT("FExecutionContextParams couldn't be cast to FRevokeContextParams.")))
	{
		return false;
	}
}

bool FModifyRule::Predicate(const UActorComponent* Component,
                            const TInstancedStruct<FExecutionContextParams>& Params) const
{
	auto* SkillTreeComponent = Cast<UActorSkillTreeComponent>(Component);
	if (!IsValid(SkillTreeComponent))
	{
		return false;
	}

	// @gdemers first, validate if our actor is in a state where they can act on a skill tree node.
	const auto* Player = SkillTreeComponent->GetTypedOuter<AAVVMCharacter>();
	if (IsValid(Player) && Player->HasPartialMatch(UAVVMGameplaySettings::GetPlayerAbilityBlockingTags()))
	{
		return false;
	}

	const auto* ModifyParams = Params.GetPtr<FModifyContextParams>();
	if (!ensureAlwaysMsgf(ModifyParams != nullptr,
	                      TEXT("FExecutionContextParams couldn't be cast to FModifyContextParams.")))
	{
		return false;
	}
}
