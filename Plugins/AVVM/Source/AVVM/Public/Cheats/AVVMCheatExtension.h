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

#include "GameFramework/CheatManager.h"

#include "AVVMCheatExtension.generated.h"

/**
*	Class description:
 *
 *	UAVVMCheatExtension is added through UGameFeatureAction_AddCheats and extend the UCheatManager without requiring deriving from
 *	the base class.
 *
 *	It exposed new console commands for testing the UAVVMNotificationSubsystem, allowing to broadcast specific tag channels and/or
 *	interface with the UAVVMPresenter Model to "inject" stub data to derive types.
 */
UCLASS(BlueprintType)
class AVVM_API UAVVMCheatExtension : public UCheatManagerExtension
{
	GENERATED_BODY()
};
