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

#include "AVVMIOUtils.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGetSetFileFromDiskComplete, const FString&);

/**
 *	Class description:
 *
 *	UAVVMIO is a blueprint function library that interface with Unreal systems to safely read/write to disk.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMIOUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void GetSetDataFromDisk(const FString& NewFileName,
	                               const FString& NewValue,
	                               const bool bShouldOverride,
	                               const FOnGetSetFileFromDiskComplete& Callback);

	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static FString GetDirFromFile(const FString& NewFileName);

	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static void GetFile(const FString& NewFileName, FString& OutValue);

	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static void ModifyFile(const FString& NewFileName, const FString& NewValue);
};
