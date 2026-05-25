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
#include "AVVMFileHelper.h"

#include "AVVMLogger.h"
#include "AVVMToolkitModule.h"
#include "AVVMToolkitSettings.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"

TStrongObjectPtr<UAVVMFileHelper> UAVVMFileHelper::gFileHelper = nullptr;

FStringView UAVVMFileHelper::Static_GetSetFileContent(const TFunction<FString()>& GenerateDefaultContent, const bool bShouldDelete)
{
	auto* FileHelper = UAVVMFileHelper::Get();
	if (IsValid(FileHelper))
	{
		return FileHelper->GetSetFileContent(UAVVMToolkitSettings::GetAppDataDirPath(),
		                                     GenerateDefaultContent,
		                                     bShouldDelete);
	}
	else
	{
		static FStringView Empty = TEXT("");
		return Empty;
	}
}

void UAVVMFileHelper::Static_Serialize(const FString& NewFileContent)
{
	auto* FileHelper = UAVVMFileHelper::Get();
	if (IsValid(FileHelper))
	{
		FileHelper->Serialize_v2(NewFileContent);
	}
}

void UAVVMFileHelper::Serialize_v2(const FString& NewFileContent)
{
	const FStringView NewPath = UAVVMToolkitSettings::GetAppDataDirPath();
	const TCHAR* FilePath = NewPath.GetData();

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!ensureAlwaysMsgf(FileManager.FileExists(FilePath), TEXT("Invalid FilePath \"%s\""), FilePath))
	{
		return;
	}

	FFileHelper::SaveStringToFile(NewFileContent, FilePath);
	MarkFileDirty();
}

UAVVMFileHelper* UAVVMFileHelper::Get()
{
	if (!gFileHelper.IsValid())
	{
		auto* Instance = NewObject<UAVVMFileHelper>();
		gFileHelper.Reset(Instance);
	}

	return gFileHelper.Get();
}

FStringView UAVVMFileHelper::GetSetFileContent(const FStringView NewFilePath,
                                               const TFunction<FString()>& GenerateDefaultContent,
                                               const bool bShouldDelete)
{
	const auto GetFileFromDisk = [](const FStringView NewPath,
	                                const TFunction<FString()>& NewGenerateDefaultContent,
	                                const bool bNewShouldDelete)
	{
		IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
		const TCHAR* FilePath = NewPath.GetData();

		FString OutFileContent;

		const bool bDoesFileExist = FileManager.FileExists(FilePath);
		if (bDoesFileExist && bNewShouldDelete)
		{
			FileManager.DeleteFile(FilePath);
		}
		else if (bDoesFileExist)
		{
			FFileHelper::LoadFileToString(OutFileContent, FilePath);
			AVVM_LOGGER_LOG(LogToolkit,
			                nullptr,
			                GetDefault<UAVVMFileHelper>(),
			                TEXT("I/O action on Disk. FFileHelper::LoadFileToString. DirPath: %s \n %s"),
			                FilePath,
			                *OutFileContent);
		}

		if (OutFileContent.IsEmpty())
		{
			OutFileContent = NewGenerateDefaultContent();
			FFileHelper::SaveStringToFile(OutFileContent, FilePath);
			AVVM_LOGGER_LOG(LogToolkit,
			                nullptr,
			                GetDefault<UAVVMFileHelper>(),
			                TEXT("I/O action on Disk. FFileHelper::SaveStringToFile. DirPath: %s \n %s"),
			                FilePath,
			                *OutFileContent);
		}

		return OutFileContent;
	};

	if (FileContent.IsEmpty() || bIsMarkedDirty || bShouldDelete)
	{
		FileContent = GetFileFromDisk(NewFilePath, GenerateDefaultContent, bShouldDelete);
		bIsMarkedDirty = false;
	}

	return FileContent;
}

void UAVVMFileHelper::MarkFileDirty()
{
	bIsMarkedDirty = true;
}
