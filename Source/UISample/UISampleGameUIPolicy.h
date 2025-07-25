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

#include "GameUIPolicy.h"

#include "UISampleGameUIPolicy.generated.h"

class APlayerController;

/**
 *	Class description:
 *
 *	UAVVMGameUIPolicy is a derived class type that extend CommonGame plugin GameUIPolicy for tracking the presence of the Primary Game Layout on screen.
 *	Doing so allow external system like the fencing system to lower post-addition or removal of the primary game layout.
 */
UCLASS()
class UISAMPLE_API UUISampleGameUIPolicy : public UGameUIPolicy
{
	GENERATED_BODY()

public:
	virtual class UWorld* GetWorld() const override;

protected:
	virtual void OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout) override;
	virtual void OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout) override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnPrimaryGameLayoutAdded(const APlayerController* NewPC) const;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnPrimaryGameLayoutRemoved(const APlayerController* NewPC) const;
};
