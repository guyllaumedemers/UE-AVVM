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

#include "UObject/Object.h"

#include "AVVMOnlineStubDataProvider.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMOnlineStubDataProvider is an impl UObject that return stub information about a property
 *	specific to {FAVVMPlayerProfile}, and initializing cached information within AVVMGameSession to simulate
 *	running a backend without the required hooks in place.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineStubDataProvider : public UObject
{
	GENERATED_BODY()
	
public:
	virtual TArray<int32> MakePropertyStubData() const PURE_VIRTUAL(MakePropertyStubData, return TArray<int32>{};);
};
