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

#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMLogger.generated.h"

/**
 *	Class Description :
 *
 *	UAVVMLoggerUtils expose a set of utility function relevant for logging data.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMLoggerUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AVVMLogger|Utils")
	static FString BP_PrintNetSource(const UObject* NetObject);

	static FStringView PrintNetSource(const UObject* NetObject);

	UFUNCTION(BlueprintCallable, Category="AVVMLogger|Utils")
	static FString PrintConnectionInfo(const UNetConnection* Connection);
};

#define AVVM_LOGGER(CategoryName, Verbosity, NetObject, TargetObject, Format, ...)\
	UE_LOG(CategoryName,\
	Verbosity,\
	TEXT("%hs line:%d. {%s} Exec. {%s} Target. Msg: %s"),\
	__FUNCTION__,\
	__LINE__,\
	UAVVMLoggerUtils::PrintNetSource(NetObject).GetData(),\
	*GetNameSafe(TargetObject),\
	*FString::Printf(Format, ##__VA_ARGS__))\

#define AVVM_LOGGER_LOG(CategoryName, NetObject, TargetObject, Format, ...)\
AVVM_LOGGER(CategoryName, Log, NetObject, TargetObject, Format, ##__VA_ARGS__)

#define AVVM_LOGGER_WARNING(CategoryName, NetObject, TargetObject, Format, ...)\
AVVM_LOGGER(CategoryName, Warning, NetObject, TargetObject, Format, ##__VA_ARGS__)

#define AVVM_LOGGER_ERROR(CategoryName, NetObject, TargetObject, Format, ...)\
AVVM_LOGGER(CategoryName, Error, NetObject, TargetObject, Format, ##__VA_ARGS__)
