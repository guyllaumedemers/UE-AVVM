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

#include "Backend/AVVMDataResolverHelper.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMOnlineBackendUtils.generated.h"

/**
 *	Class description:
 *
 *	UAVVMOnlineBackendUtils expose a set of utility function relevant for the backend service. It
 *	offers reusable api for data types defined under AVVMOnlinePlayer.
 */
UCLASS()
class AVVMONLINE_API UAVVMOnlineBackendUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static TArray<int32> GetElementDependencies(const UObject* WorldContextObject,
	                                            const int32 ElementUniqueId,
	                                            const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static bool CompareSet(const TArray<int32>& Lhs,
	                       const TArray<int32>& Rhs);

	UFUNCTION(BlueprintCallable, Category="AVVM|Online")
	static void Submit(const int32 TargetUniqueId, const FString& NewProfile);
};
