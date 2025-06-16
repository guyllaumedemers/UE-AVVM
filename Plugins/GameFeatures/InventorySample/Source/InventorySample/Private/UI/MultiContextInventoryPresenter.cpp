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
#include "UI/MultiContextInventoryPresenter.h"

#include "AbilitySystemComponent.h"
#include "AVVMGameplayUtils.h"
#include "GameFramework/PlayerState.h"

AActor* UMultiContextInventoryPresenter::GetOuterKey() const
{
	return GetTypedOuter<APlayerState>();
}

void UMultiContextInventoryPresenter::SafeBeginPlay()
{
	Super::SafeBeginPlay();

	const auto* PlayerState = GetTypedOuter<APlayerState>();
	if (!ensureAlwaysMsgf(IsValid(PlayerState), TEXT("UMultiContextInventoryPresenter isn't owned by a valid APlayerState!")))
	{
		return;
	}

	const bool bIsActorLocallyControlled = UAVVMGameplayUtils::IsLocallyControlled(PlayerState);
	if (bIsActorLocallyControlled)
	{
		OwnerASC = PlayerState->GetComponentByClass<UAbilitySystemComponent>();
	}
}

void UMultiContextInventoryPresenter::SafeEndPlay()
{
	Super::SafeEndPlay();
	OwnerASC.Reset();
}

void UMultiContextInventoryPresenter::StartPresenting()
{
	Super::StartPresenting();

	auto* AbilityComponent = OwnerASC.Get();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AddReplicatedLooseGameplayTags(GrantAbilityTags);
	}
}

void UMultiContextInventoryPresenter::StopPresenting()
{
	Super::StopPresenting();

	auto* AbilityComponent = OwnerASC.Get();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveReplicatedLooseGameplayTags(GrantAbilityTags);
	}
}
