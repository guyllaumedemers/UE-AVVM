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
#include "AVVMIOUtils.h"

void UAVVMIOUtils::GetSetDataFromDisk(const FString& NewFileName,
                                      const FString& NewValue,
                                      const bool bShouldOverride,
                                      const FOnGetSetFileFromDiskComplete& Callback)
{
	if (bShouldOverride)
	{
		ModifyFile(NewFileName, NewValue);
	}

	FString OutFileValue;
	GetFile(NewFileName, OutFileValue);

	if (OutFileValue.IsEmpty())
	{
		static const FStringView Tokens = TEXT("");
		ensureAlwaysMsgf(FFileHelper::SaveStringToFile(NewFileName, Tokens.GetData()),
		                 TEXT("Failed to write to file \"%s\"."),
		                 *NewFileName);

		Callback.Broadcast(Tokens.GetData());
	}
	else
	{
		Callback.Broadcast(OutFileValue);
	}
}

FString UAVVMIOUtils::GetDirFromFile(const FString& NewFileName)
{
	return TEXT("");
}

void UAVVMIOUtils::GetFile(const FString& NewFileName,
                           FString& OutValue)
{
	const bool bDoesFileExist = IPlatformFile::GetPlatformPhysical().FileExists(*NewFileName);
	if (!bDoesFileExist)
	{
		const FString NewDir = GetDirFromFile(NewFileName);
		ensureAlwaysMsgf(IPlatformFile::GetPlatformPhysical().CreateDirectory(*NewDir),
		                 TEXT("Failed to create file \"%s\"."),
		                 *NewFileName);
	}

	ensureAlwaysMsgf(FFileHelper::LoadFileToString(OutValue, &IPlatformFile::GetPlatformPhysical(), *NewFileName),
	                 TEXT("Failed to read from file \"%s\"."),
	                 *NewFileName);
}

void UAVVMIOUtils::ModifyFile(const FString& NewFileName,
                              const FString& NewValue)
{
	const bool bDoesFileExist = IPlatformFile::GetPlatformPhysical().FileExists(*NewFileName);
	if (!bDoesFileExist)
	{
		const FString NewDir = GetDirFromFile(NewFileName);
		ensureAlwaysMsgf(IPlatformFile::GetPlatformPhysical().CreateDirectory(*NewDir),
		                 TEXT("Failed to create file \"%s\"."),
		                 *NewFileName);
	}

	ensureAlwaysMsgf(FFileHelper::SaveStringToFile(NewFileName, *NewValue),
	                 TEXT("Failed to write to file \"%s\"."),
	                 *NewFileName);
}
