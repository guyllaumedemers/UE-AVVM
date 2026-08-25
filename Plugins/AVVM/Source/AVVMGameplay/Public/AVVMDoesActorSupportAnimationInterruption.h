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

#include "UObject/Interface.h"

#include "AVVMDoesActorSupportAnimationInterruption.generated.h"

/**
 *	Class description:
 *	
 *	IAVVMDoesActorSupportAnimationInterruption is an interface to be impl in an actor class that expect
 *	running animation transition in a predicted fashion, example : equip/unequip, and that may require stalling/pausing animations
 *	during possible state desync between server-client.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMDoesActorSupportAnimationInterruption : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMDoesActorSupportAnimationInterruption
{
	GENERATED_BODY()

public:
	// @gdemers Start an animation at pose 0, no matter its progression.
	UFUNCTION(BlueprintNativeEvent)
	void Restart();
	virtual void Restart_Implementation() PURE_VIRTUAL(Restart_Implementation, return;);

	// @gdemers Pause a running animation thats not complete.
	UFUNCTION(BlueprintNativeEvent)
	void Pause();
	virtual void Pause_Implementation() PURE_VIRTUAL(Pause_Implementation, return;);
	
	// @gdemers Resume an animation that was paused, and later reselected.
	UFUNCTION(BlueprintNativeEvent)
	void Resume();
	virtual void Resume_Implementation() PURE_VIRTUAL(Resume_Implementation, return;);

	// @gdemers Flush a Paused animation.
	UFUNCTION(BlueprintNativeEvent)
	void Flush();
	virtual void Flush_Implementation() PURE_VIRTUAL(Flush_Implementation, return;);
};
