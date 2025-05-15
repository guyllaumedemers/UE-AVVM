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

#include "AVVMSampleRuntimeModule.h"
#include "DataRegistryTypes.h"
#include "Cheats/AVVMCheatData.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CheatManager.h"

#if WITH_AVVM_DEBUGGER
#include "AVVMDebugger.h"
#endif

#include "AVVMCheatExtension.generated.h"

/**
 *	Class description:
 *
 *	UAVVMCheatExtension is added through UGameFeatureAction_AddCheats and extend the UCheatManager without requiring deriving from
 *	the base class.
 *
 *	It exposed new console commands for testing the UAVVMNotificationSubsystem, allowing to broadcast specific tag channels and/or
 *	interface with the UAVVMPresenter Model to "inject" stub data to derive types.
 */
UCLASS(BlueprintType)
class AVVMSAMPLERUNTIME_API UAVVMCheatExtension : public UCheatManagerExtension
#if WITH_AVVM_DEBUGGER
                                                  , public IAVVMImGuiDescriptor
#endif
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	// @gdemers cheat function which interface with the UAVVMNotificationSubsystem. It "inject"
	// a Payload referenced via Data Registry Id.
	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.NotifyTagChannel.Payload")
	void NotifyChannelWithPayload(const FString& TagChannel,
	                              const FString& PayloadRegistryId);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.NotifyTagChannel.NoPayload")
	void NotifyChannelNoPayload(const FString& TagChannel);

#if WITH_AVVM_DEBUGGER
	virtual void Draw() override;
#endif

protected:
	void OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result);
	void OnSoftObjectAcquired();

	void AddStreamableHandle(const FDataRegistryId& RegistryId,
	                         const TSharedPtr<FStreamableHandle> StreamableHandle);

	void ClearAllStreamableHandle();

	void PushRequest(const TPair<FDataRegistryId, FGameplayTag>& Request);
	void PopRequest();
	void ClearAllRequests();

	TInstancedStruct<FAVVMCheatData> GetPayload(const TSharedPtr<FStreamableHandle> StreamableHandle);

#if WITH_AVVM_DEBUGGER
	inline const char* LazyGatherTagChannels(bool& bForceGathering) const;
	inline const char* LazyGatherRegistryIds(bool& bForceGathering) const;

	inline FString GetIndexedString(const char* ConcatString,
	                                const int32 Index) const;

	inline void HandleComboBoxLinkage(const bool bIsComboBoxLinked,
	                                  const int32& TagChannelIndex,
	                                  int32& OutRegistryIndex);
#endif

	// @gdemers handle data registry/gameplay tag changes at runtime. (most-likely triggered from GFP)
	void OnDataRegistrySubsystemChanged();
	void OnGameplayTagTreeChanged();

	TMap<FDataRegistryId, TSharedPtr<FStreamableHandle>> StreamableHandles;
	TArray<TPair<FDataRegistryId, FGameplayTag>> NotificationRequests;
	bool bHasRegistriesChanged = false;
	bool bHasTagChanged = false;
};
