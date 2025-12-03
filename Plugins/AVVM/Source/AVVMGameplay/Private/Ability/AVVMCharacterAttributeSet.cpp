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
#include "Ability/AVVMCharacterAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UAVVMCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMCharacterAttributeSet, Movement_JumpHeight, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMCharacterAttributeSet, Movement_WalkSpeed, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMCharacterAttributeSet, Movement_SprintSpeed, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMCharacterAttributeSet, Movement_DashDistance, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMCharacterAttributeSet, Movement_RollDistance, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAVVMCharacterAttributeSet, Movement_SlideDistance, Params);
}

void UAVVMCharacterAttributeSet::Init()
{
	Super::Init();
}
