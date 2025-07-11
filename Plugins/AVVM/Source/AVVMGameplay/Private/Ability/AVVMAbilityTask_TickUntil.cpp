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
#include "Ability/AVVMAbilityTask_TickUntil.h"

UAVVMAbilityTask_TickUntil::UAVVMAbilityTask_TickUntil(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UAVVMAbilityTask_TickUntil* UAVVMAbilityTask_TickUntil::TickUntil(UGameplayAbility* OwningAbility, const bool bTestAlreadyReleased)
{
	UAVVMAbilityTask_TickUntil* Task = NewAbilityTask<UAVVMAbilityTask_TickUntil>(OwningAbility);
	Task->bTestInitialState = bTestAlreadyReleased;
	return Task;
}

void UAVVMAbilityTask_TickUntil::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	OnTick.Broadcast(DeltaTime);
}

void UAVVMAbilityTask_TickUntil::Activate()
{
	Super::Activate();

	if (!IsLocallyControlled())
	{
		EndTask();
		return;
	}

	if (bTestInitialState)
	{
		const auto* GameplayAbility = Cast<UGameplayAbility>(TaskOwner.Get());
		if (IsValid(GameplayAbility) && !GameplayAbility->IsActive())
		{
			EndTask();
		}
	}
}
