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

#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"

#include "AVVMOnlinePlayerSubsystem.generated.h"

struct FAVVMNotificationPayload;

/**
 *	Class description:
 *	
 *	UAVVMOnlinePlayerSubsystem is a subsystem that allow clients retrieval of information about players account, and
 *	parse backend information for display purposes.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMOnlinePlayerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	static FString Static_GetPlayerProfile(const UWorld* World,
	                                       const int32 ProfileId);

	UFUNCTION(BlueprintCallable)
	static FString Static_GetPlayerPreset(const UWorld* World,
	                                      const int32 PresetId);

	UFUNCTION(BlueprintCallable)
	static TArray<int32> Static_GetPlayerEquippedItems(const UWorld* World,
	                                                   const int32 ProfileId);

protected:
	static UAVVMOnlinePlayerSubsystem* Get(const UWorld* World);
	FString GetPlayerProfile(const int32 ProfileId) const;
	FString GetPlayerPreset(const int32 PresetId) const;
	TArray<int32> GetPlayerEquippedItems(const int32 ProfileId) const;

	UFUNCTION(CallInEditor)
	void OnPlayerStateAddedOrRemoved(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload);

	UFUNCTION()
	void OnPlayerStateAdded(const APlayerState* NewPlayerState);

	UFUNCTION()
	void OnPlayerStateRemoved(const APlayerState* NewPlayerState);
	
	UPROPERTY(Transient)
	TMap<int32/*ProfileId*/, FString/*FAVVMPlayerProfile*/> PlayerProfiles;

	UPROPERTY(Transient)
	TMap<int32/*PresetId*/, FString/*FAVVMPlayerPreset*/> PlayerPresets;
};
