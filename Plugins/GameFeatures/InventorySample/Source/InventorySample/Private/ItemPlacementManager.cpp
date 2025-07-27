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
#include "ItemPlacementManager.h"

bool UItemPlacementManager::ShouldCreateSubsystem(UObject* Outer) const
{
	if (IsRunningDedicatedServer())
	{
		return false;
	}

	const UWorld* PieOrGameWorld = Cast<UWorld>(Outer);
	if (IsValid(PieOrGameWorld))
	{
		const bool bIsGameClient = PieOrGameWorld->IsGameWorld();
		return bIsGameClient;
	}
	else
	{
		return false;
	}
}

UItemPlacementManager* UItemPlacementManager::GetSubsystem(const ULocalPlayer* NewLocalPlayer)
{
	return IsValid(NewLocalPlayer) ? NewLocalPlayer->GetSubsystem<UItemPlacementManager>() : nullptr;
}

void UItemPlacementManager::SetupItemPlacements(const TArray<UItemObjectViewModel*>& NewViewModels)
{
	const bool bDoesRequireBackendSync = DoesRequireBackendSync();
	if (bDoesRequireBackendSync)
	{
		FOnBackSyncCompleted OnSyncCompleted;
		OnSyncCompleted.AddUObject(this, &UItemPlacementManager::OnBackendSyncComplete, NewViewModels);
		SyncBackend(OnSyncCompleted);
	}
	else
	{
		// @gdemers path for when there's no backend.
		OnBackendSyncComplete(false, TEXT(""), NewViewModels);
	}
}

FString UItemPlacementManager::GetFileName() const
{
	const auto* OwningOuter = GetTypedOuter<ULocalPlayer>();

	FStringFormatNamedArguments Args;
	if (IsValid(OwningOuter))
	{
		Args.Add(TEXT("ProjectUserConfigFilename"), GetProjectUserConfigFilename());
		Args.Add(TEXT("PlayerId"), OwningOuter->GetName());
	}

	return IsValid(OwningOuter) ? FString::Format(TEXT("{ProjectUserConfigFilename}/{PlayerId}_InventoryPreset.ini"), Args) : TEXT("Unknown");
}

bool UItemPlacementManager::DoesRequireBackendSync() const
{
	return true;
}

void UItemPlacementManager::SyncBackend(const FOnBackSyncCompleted& Callback)
{
	Callback.Broadcast(true, TEXT(""));
}

void UItemPlacementManager::OnBackendSyncComplete(const bool bWasSuccess,
                                                  const FString& NewFileValue,
                                                  TArray<UItemObjectViewModel*> NewViewModels)
{
	const FString NewFile = GetFileName();

	if (bWasSuccess)
	{
		ModifyFile(NewFile, NewFileValue);
	}

	FString OutFileValue = TEXT("");
	GetFile(NewFile, OutFileValue);

	if (OutFileValue.IsEmpty())
	{
		const FString TokenizedValues = TEXT("") /*Tokenized View Model id*/;
		ensureAlwaysMsgf(WriteFileOnDisk(NewFile, TokenizedValues),
		                 TEXT("Failed to write to file \"%s\"."),
		                 *NewFile);

		RefreshTokens(TokenizedValues, NewViewModels);
	}
	else
	{
		RefreshTokens(OutFileValue, NewViewModels);
	}
}

void UItemPlacementManager::GetFile(const FString& NewFile,
                                    FString& OutValue)
{
	const bool bDoesFileExist = DoesFileOnDiskExist(NewFile);
	if (!bDoesFileExist)
	{
		ensureAlwaysMsgf(CreateFileOnDisk(NewFile),
		                 TEXT("Failed to create file \"%s\"."),
		                 *NewFile);
	}

	ensureAlwaysMsgf(ReadFileOnDisk(NewFile, OutValue),
	                 TEXT("Failed to read from file \"%s\"."),
	                 *NewFile);
}

void UItemPlacementManager::ModifyFile(const FString& NewFile,
                                       const FString& NewFileValue) const
{
	const bool bDoesFileExist = DoesFileOnDiskExist(NewFile);
	if (!bDoesFileExist)
	{
		ensureAlwaysMsgf(CreateFileOnDisk(NewFile),
		                 TEXT("Failed to create file \"%s\"."),
		                 *NewFile);
	}

	ensureAlwaysMsgf(WriteFileOnDisk(NewFile, NewFileValue),
	                 TEXT("Failed to write to file \"%s\"."),
	                 *NewFile);
}

bool UItemPlacementManager::DoesFileOnDiskExist(const FString& NewFile) const
{
	// TODO @gdemers setup io stream to validate
	return false;
}

bool UItemPlacementManager::CreateFileOnDisk(const FString& NewFile) const
{
	// TODO @gdemers setup io stream to create
	return true;
}

bool UItemPlacementManager::ReadFileOnDisk(const FString& NewFile,
                                           FString& OutValue) const
{
	// TODO @gdemers setup io stream to read
	return true;
}

bool UItemPlacementManager::WriteFileOnDisk(const FString& NewFile,
                                            const FString& NewValue) const
{
	// TODO @gdemers setup io stream to write
	return true;
}

void UItemPlacementManager::RefreshTokens(const FString& NewValue,
                                          const TArray<UItemObjectViewModel*>& NewViewModels)
{
	// TODO @gdemers refresh TokenGroups. NewValue return {Tag}{{A.Id},{B.Id}, etc...}
}
