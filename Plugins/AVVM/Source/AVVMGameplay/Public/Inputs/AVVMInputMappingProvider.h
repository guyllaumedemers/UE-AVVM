﻿//Copyright(c) 2025 gdemers
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

#include "UObject/Interface.h"

#include "AVVMInputMappingProvider.generated.h"

class UInputMappingContext;

/**
 *	Class description:
 *
 *	UAVVMInputMappingProvider return access to an Input Mapping Context specific to an Actor type.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMInputMappingProvider : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMInputMappingProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	const UInputMappingContext* GetInputMappingContext() const;
	virtual const UInputMappingContext* GetInputMappingContext_Implementation() const PURE_VIRTUAL(GetInputMappingContext_Implementation, return nullptr;);
};
