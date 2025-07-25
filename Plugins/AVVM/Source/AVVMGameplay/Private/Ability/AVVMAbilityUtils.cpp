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
#include "Ability/AVVMAbilityUtils.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

const AActor* UAVVMAbilityUtils::GetEffectCauser(const UAbilitySystemComponent* NewAbilitySystemComponent,
                                                 const FGameplayTagContainer& NewGEQueryTags)
{
	if (!IsValid(NewAbilitySystemComponent))
	{
		return nullptr;
	}

	const auto GEQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(NewGEQueryTags);
	const TArray<FActiveGameplayEffectHandle> GEActiveHandles = NewAbilitySystemComponent->GetActiveGameplayEffects().GetActiveEffects(GEQuery);
	if (GEActiveHandles.IsEmpty())
	{
		return nullptr;
	}

	ensureAlwaysMsgf(GEActiveHandles.Num() == 1, TEXT("Multiple Matches Found! There should only ever be one match!"));

	const FActiveGameplayEffect* GEActive = NewAbilitySystemComponent->GetActiveGameplayEffect(GEActiveHandles[0]);
	if (GEActive == nullptr)
	{
		return nullptr;
	}

	const FGameplayEffectContextHandle& GECtx = GEActive->Spec.GetEffectContext();
	return GECtx.GetEffectCauser();
}
