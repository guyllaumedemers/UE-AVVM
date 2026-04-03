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

#include "AVVMPlayerRule.generated.h"

/**
 *	Class description:
 *
 *	UAVVMPlayerAcceptanceRule is a UObject instance that run validation on incoming player joining gameplay. Based on server authority,
 *	we can approve, or reject this newly recorded player.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMPlayerAcceptanceRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
	// @gdemers validate information about player based on the referenced UniqueNetId.
	UFUNCTION(BlueprintNativeEvent)
	bool Predicate_IsExpectedUniqueNetId(const FUniqueNetIdRepl& UniqueId) const;
	virtual bool Predicate_IsExpectedUniqueNetId_Implementation(const FUniqueNetIdRepl& UniqueId) const PURE_VIRTUAL(Predicate_IsExpectedUniqueNetId_Implementation, return false;);
};
