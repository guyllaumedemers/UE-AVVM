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
#include "AVVMSaveGame.h"

#include "AVVMFileHelper.h"
#include "AVVMLogger.h"
#include "AVVMToolkitModule.h"
#include "AVVMToolkitUtils.h"
#include "Kismet/GameplayStatics.h"

FStringView UAVVMSaveGame::Static_GetSetFileContent(const FName PayloadType,
                                                    const TFunction<FString()>& GenerateDefaultContent,
                                                    const bool bShouldDelete)
{
	auto* SaveGame = UAVVMFileHelper::Static_GetSetSaveGame();
	return ensureAlwaysMsgf(IsValid(SaveGame), TEXT("Invalid SaveGame")) ? SaveGame->GetSetFileContent(PayloadType, GenerateDefaultContent, bShouldDelete) : TEXT("");
}

void UAVVMSaveGame::Static_Serialize(const FName PayloadType,
                                     const FString& NewPayload)
{
	auto* SaveGame = UAVVMFileHelper::Static_GetSetSaveGame();
	if (ensureAlwaysMsgf(IsValid(SaveGame), TEXT("Invalid SaveGame")))
	{
		SaveGame->Serialize_v2(PayloadType, NewPayload);
	}
}

void UAVVMSaveGame::HandlePreSave()
{
	Super::HandlePreSave();
	PrevPayloadPerType = CurrPayloadPerType;

	const double Now = UAVVMToolkitUtils::GetServerWorldTime(this);
	TotalPlayTime += (Now - SessionStartTime);
	SessionStartTime = Now;
}

void UAVVMSaveGame::HandlePostLoad()
{
	Super::HandlePostLoad();
	CurrPayloadPerType = PrevPayloadPerType;

	const double Now = UAVVMToolkitUtils::GetServerWorldTime(this);
	SessionStartTime = Now;
}

void UAVVMSaveGame::HandlePostSave(bool bSuccess)
{
	Super::HandlePostSave(bSuccess);
	bIsMarkedDirty = (static_cast<int32>(bIsMarkedDirty) & ~static_cast<int32>(bSuccess));
}

FStringView UAVVMSaveGame::GetSetFileContent(const FName PayloadType,
                                             const TFunction<FString()>& GenerateDefaultContent,
                                             const bool bShouldDelete)
{
	FString& FileContent = CurrPayloadPerType.FindOrAdd(PayloadType);
	if (FileContent.IsEmpty() || bShouldDelete)
	{
		FileContent = GenerateDefaultContent();
		MarkFileDirty();
	}

	if (bIsMarkedDirty)
	{
		// serialize
		AVVM_LOGGER_LOG(LogToolkit,
		                nullptr,
		                GetDefault<UAVVMFileHelper>(),
		                TEXT("I/O action on Disk. FFileHelper::SaveStringToFile. Save Game Slot: %s \n %s"),
		                *GetSaveSlotName(),
		                *FileContent);

#if !WITH_EDITOR
		ensureAlwaysMsgf(SaveGameToSlotForLocalPlayer(), TEXT("Failed to save game slot."));
#endif
	}

	return FileContent;
}

void UAVVMSaveGame::Serialize_v2(const FName PayloadType,
                                 const FString& NewPayload)
{
	const bool bDoesContains = CurrPayloadPerType.Contains(PayloadType);
	if (!ensureAlwaysMsgf(bDoesContains, TEXT("Invalid Payload Type")))
	{
		return;
	}

	FString& OutResult = CurrPayloadPerType[PayloadType];
	OutResult = NewPayload;
	MarkFileDirty();
}

void UAVVMSaveGame::MarkFileDirty()
{
	bIsMarkedDirty = true;
}
