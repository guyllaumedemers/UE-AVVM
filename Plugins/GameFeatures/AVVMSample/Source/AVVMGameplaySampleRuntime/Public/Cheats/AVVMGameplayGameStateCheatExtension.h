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

#include "GameFramework/CheatManager.h"

#ifdef UE_ENABLE_AVVM_DEBUGGER
#include "AVVMDebugger.h"
#endif

#include "AVVMGameplayGameStateCheatExtension.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMGameplayGameStateCheatExtension is a CheatExtension, added via GFP, that expose a set of new console commands to test recurrent gameplay features.
 *	See IAVVMQuicktimeEventGameStateInterface for project implementation details!
 */
UCLASS()
class AVVMGAMEPLAYSAMPLERUNTIME_API UAVVMGameplayGameStateCheatExtension : public UCheatManagerExtension
#if WITH_AVVM_DEBUGGER
                                                                           ,
                                                                           public IAVVMImGuiDescriptor
#endif
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.Disconnect")
	void Disconnect();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.Connect")
	void Connect();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.Win")
	void Win();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.Lose")
	void Lose();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.Kill")
	void Kill();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.Killstreak")
	void Killstreak();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.CaptureObjective")
	void CaptureObjective();

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameState.DiscoverArea")
	void DiscoverArea();

#if WITH_AVVM_DEBUGGER
	virtual void Draw() override;
#endif
};
