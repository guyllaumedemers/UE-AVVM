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

#include "AVVMGameModeAdditiveCheatExtension.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMGameModeAdditiveCheatExtension is a CheatExtension, added via GFP, that expose a set of new console commands to test recurrent gameplay features
 *	specific to the AVVMGameModeAdditive system.
 */
UCLASS()
class AVVMSAMPLEDEBUG_API UAVVMGameModeAdditiveCheatExtension : public UAVVMClientExecutorCheatExtension,
                                                                public IAVVMImGuiDescriptor
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameModeAdditive.Create")
	void Create(const FString& GameModeAdditiveClassAssetName);

	UFUNCTION(Exec, BlueprintCallable, Category="AVVM|Cheats", DisplayName="AVVM.GameModeAdditive.Destroy")
	void Destroy(const FString& GameModeAdditiveClassAssetName);

	virtual void Draw() override;

protected:
	const char* LazyGatherGameModeAdditiveClasses(bool& bForceGathering) const;

	FString GetIndexedString(const char* ConcatString,
	                         const int32 Index) const;

	// @gdemers handle data registry/gameplay tag changes at runtime. (most-likely triggered from GFP)
	void OnDataRegistrySubsystemChanged();

	bool bHasRegistriesChanged = false;
};
