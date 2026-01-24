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
#include "InventoryFileHelper.h"

#include "InventorySettings.h"

TStrongObjectPtr<UInventoryFileHelper> UInventoryFileHelper::gInventoryFileHelper = nullptr;

FStringView UInventoryFileHelper::Static_GetSetFileContent()
{
	auto* FileHelper = UInventoryFileHelper::Get();
	if (IsValid(FileHelper))
	{
		return FileHelper->GetSetFileContent(UInventorySettings::GetAppDataDirPath());
	}
	else
	{
		static FStringView Empty;
		return Empty;
	}
}

void UInventoryFileHelper::Static_MarkFileDirty()
{
	auto* FileHelper = UInventoryFileHelper::Get();
	if (IsValid(FileHelper))
	{
		FileHelper->MarkFileDirty();
	}
}

UInventoryFileHelper* UInventoryFileHelper::Get()
{
	if (!gInventoryFileHelper.IsValid())
	{
		auto* Instance = NewObject<UInventoryFileHelper>();
		gInventoryFileHelper.Reset(Instance);
	}

	return gInventoryFileHelper.Get();
}

FStringView UInventoryFileHelper::GetSetFileContent(const FStringView NewFilePath)
{
	const auto GetFileFromDisk = [](const FStringView NewPath)
	{
		IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
		const TCHAR* FilePath = NewPath.GetData();

		FString OutFileContent;
		if (ensureAlwaysMsgf(FileManager.FileExists(FilePath), TEXT("Invalid FilePath \"%s\""), FilePath))
		{
			FFileHelper::LoadFileToString(OutFileContent, FilePath);
		}

		return OutFileContent;
	};

	if (FileContent.IsEmpty() || bIsMarkedDirty)
	{
		FileContent = GetFileFromDisk(NewFilePath);
		bIsMarkedDirty = false;
	}

	return FileContent;
}

void UInventoryFileHelper::MarkFileDirty()
{
	bIsMarkedDirty = true;
}
