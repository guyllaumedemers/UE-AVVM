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

#include "FenceCheatExtension.generated.h"

class UActorFenceComponent;

/**
*	Class description:
 *
 *	UFenceCheatExtension expose basic functionalities for interfacing with the Fencing system.
 */
UCLASS()
class FENCINGSAMPLE_API UFenceCheatExtension : public UCheatManagerExtension
#if WITH_AVVM_DEBUGGER
                                               ,
                                               public IAVVMImGuiDescriptor
#endif
{
	GENERATED_BODY()

public:
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

#if WITH_AVVM_DEBUGGER
	virtual void Draw() override;
#endif

protected:
#if WITH_AVVM_DEBUGGER
	const char* LazyGatherFenceInstances(bool& bForceGathering);
#endif

	UFUNCTION()
	void OnFencesCountModified();

	void LowerFence(const int32 NewIndex);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<const UActorFenceComponent>> FenceComponents;

	bool bHasFencesChanged = false;
};
