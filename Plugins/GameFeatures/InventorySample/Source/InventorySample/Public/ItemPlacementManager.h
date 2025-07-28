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

#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"

#include "ItemPlacementManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBackendSyncCompleted, const bool bWasSuccess, const FString& NewFile);

class UItemObjectViewModel;

/**
 *	Class description:
 *
 *	UItemPlacementManager is a local player subsystem that manages object layout/presentation based on the owning player. It serializes to disk
 *	and retrieves from backend information about object placement.
 */
UCLASS()
class UItemPlacementManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UFUNCTION(BlueprintCallable)
	static UItemPlacementManager* GetSubsystem(const ULocalPlayer* NewLocalPlayer);

	UFUNCTION(BlueprintCallable)
	void SetupItemPlacements(const TArray<UItemObjectViewModel*>& NewViewModels);

protected:
	virtual bool DoesRequireBackendSync() const;
	virtual void SyncBackend(const FOnBackendSyncCompleted& Callback);

	UFUNCTION()
	void OnBackendSyncComplete(const bool bWasSuccess, const FString& NewFileValue, TArray<UItemObjectViewModel*> NewViewModels);

	virtual FString GetFileName() const;

	struct FItemGroup
	{
		TArray<TWeakObjectPtr<const UItemObjectViewModel>> Items;
	};

	TMap<FGameplayTag, FItemGroup> TokenGroups;

private:
	void GetFile(const FString& NewFile, FString& OutValue);
	FString GetDirFromFile(const FString& NewFile) const;
	void ModifyFile(const FString& NewFile, const FString& NewFileValue) const;
	bool DoesFileOnDiskExist(const FString& NewFile) const;
	bool CreateFileDirOnDisk(const FString& NewDir) const;
	bool ReadFileOnDisk(const FString& NewFile, FString& OutValue) const;
	bool WriteFileOnDisk(const FString& NewFile, const FString& NewValue) const;
	void RefreshTokens(const FString& NewValue, const TArray<UItemObjectViewModel*>& NewViewModels);
};
