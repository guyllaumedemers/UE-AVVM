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

#include "AVVMIOUtils.h"
#include "InventorySample.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "UI/ItemObjectViewModel.h"

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

void UItemPlacementManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto* Outer = GetTypedOuter<ULocalPlayer>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Initializing \"%s\" on Outer \"%s\"."),
	       *UItemPlacementManager::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter = Outer;
}

void UItemPlacementManager::Deinitialize()
{
	Super::Deinitialize();

	const ULocalPlayer* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Deinitializing \"%s\" on Outer \"%s\"."),
	       *UItemPlacementManager::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter.Reset();
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
		FOnBackendSyncCompleted OnSyncCompleted;
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
	const auto* Outer = OwningOuter.Get();

	FStringFormatNamedArguments Args;
	if (IsValid(Outer))
	{
		Args.Add(TEXT("ProjectUserConfigFilename"), GetProjectUserConfigFilename());
		Args.Add(TEXT("PlayerId"), Outer->GetName());
		return FString::Format(TEXT("{ProjectUserConfigFilename}/{PlayerId}_InventoryPreset.ini"), Args);
	}

	return TEXT("Unknown");
}

bool UItemPlacementManager::DoesRequireBackendSync() const
{
	return true;
}

void UItemPlacementManager::SyncBackend(const FOnBackendSyncCompleted& Callback)
{
	// TODO @gdemers backend api should require passing in the function signature the id of the inventory
	// specific to the owning local player we are trying to configure here!
	Callback.Broadcast(true, TEXT(""));
}

void UItemPlacementManager::OnBackendSyncComplete(const bool bWasSuccess,
                                                  const FString& NewFileValue,
                                                  TArray<UItemObjectViewModel*> NewViewModels)
{
	const FString NewFileName = GetFileName();

	FOnGetSetFileFromDiskComplete Callback;
	Callback.AddUObject(this, &UItemPlacementManager::RefreshTokens, NewViewModels);
	UAVVMIOUtils::GetSetDataFromDisk(NewFileName, NewFileValue, bWasSuccess, Callback);
}

void UItemPlacementManager::RefreshTokens(const FString& NewValue,
                                          TArray<UItemObjectViewModel*> NewViewModels)
{
	const auto* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogInventorySample,
	       Log,
	       TEXT("Refreshing Tokens for player \"%s\". New Value : \"%s\"."),
	       *Outer->GetName(),
	       *NewValue);
}
