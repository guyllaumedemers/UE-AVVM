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

#include "AVVMGameplayPlayerStateCheatExtension.generated.h"

/**
 *	Class description:
 *
 *	UAVVMGameplayPlayerStateCheatExtension is added through UGameFeatureAction_AddCheats and extend the UCheatManager without requiring deriving from
 *	the base class.
 *
 *	It exposed new console commands for testing the QuicktimeEvents specific to the Player State.
 */
UCLASS()
class AVVMGAMEPLAYSAMPLERUNTIME_API UAVVMGameplayPlayerStateCheatExtension : public UCheatManagerExtension
#if WITH_AVVM_DEBUGGER
                                                                             ,
                                                                             public IAVVMImGuiDescriptor
#endif
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.Damage")
	void Damage(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.Heal")
	void Heal(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.Die")
	void Die(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.Raise")
	void Raise(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.Stun")
	void Stun(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.Exhaust")
	void Exhaust(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.StartCasting")
	void StartCasting(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.StopCasting")
	void StopCasting(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.EarnMoney")
	void EarnMoney(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.SpendMoney")
	void SpendMoney(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.StartInteractingWithWorld")
	void StartInteractingWithWorld(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.StopInteractingWithWorld")
	void StopInteractingWithWorld(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.ConsumeItem")
	void ConsumeItem(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.StartTalkingToNpc")
	void StartTalkingToNpc(const int32 PlayerIndex = 0);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.PlayerState.StopTalkingToNpc")
	void StopTalkingToNpc(const int32 PlayerIndex = 0);

#if WITH_AVVM_DEBUGGER
	virtual void Draw() override;
#endif
};
