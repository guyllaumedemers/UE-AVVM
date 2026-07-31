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

#include "AVVMClientExecutorCheatExtension.h"
#include "AVVMImGuiModule.h"

#include "AVVMOnlineCheatExtension.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMOnlineCheatExtension is a CheatExtension, added via GFP, that expose a set of new console commands to test recurrent online/backend features. In this case,
 *	we are providing the required api to inject runtime data in place of a backend service.
 */
UCLASS()
class AVVMSAMPLEDEBUG_API UAVVMOnlineCheatExtension : public UAVVMClientExecutorCheatExtension,
                                                      public IAVVMImGuiDescriptor
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.ONline.MakePlayerProfile")
	void MakePlayerProfile(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.ONline.MakePlayerPreset")
	void MakePlayerPreset(const int32 PlayerIndex = 0);

	virtual void Draw() override;

protected:
	void GetPayloads(TArray<FString>& OutProfiles, TArray<FString>& OutPresets) const;
	TStringView<char> GetProfiles(const TArray<FString>& Profiles) const;
	TStringView<char> GetPresets(const TArray<FString>& Presets) const;
};
