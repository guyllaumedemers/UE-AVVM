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

#include "CommonInputSubsystem.h"
#include "CommonInputPreprocessor.h"

#include "AVVMCommonInputPreprocessor.generated.h"

class FAVVMCommonInputPreprocessor;

/**
 *	Class description:
 *	
 *	UAVVMCommonInputSubsystem is a derivation from CommonUI input overrides so we can further extend input processing based on project requirements
 *	for Delta tracking of Mouse wheel events, among other things.
 */
UCLASS(config=Game)
class AVVM_API UAVVMCommonInputSubsystem : public UCommonInputSubsystem
{
	GENERATED_BODY()

public:
	static const FAVVMCommonInputPreprocessor* Static_GetMouseProcessor(const ULocalPlayer* LocalPlayer);
	static float Static_GetMouseWheelDelta(const ULocalPlayer* LocalPlayer);

	virtual TSharedPtr<FCommonInputPreprocessor> MakeInputProcessor() override;

protected:
	const FAVVMCommonInputPreprocessor* GetMouseProcessor() const;
	float GetMouseWheelDelta() const;

	UPROPERTY(config, EditAnywhere)
	float InputFlushThreshold{1.f};
};

/**
 *	Class description:
 *	
 *	FAVVMCommonInputPreprocessor is a preprocessor hijacking FSlateApplication received input events (before UI, and before Game - i.e straight
 *	from the Os input events api).
 *	
 *	This object is used for managing mouse related delta regarding Mouse Wheel events, among other things.
 */
class AVVM_API FAVVMCommonInputPreprocessor : public FCommonInputPreprocessor
{
public:
	FAVVMCommonInputPreprocessor() = default;
	explicit FAVVMCommonInputPreprocessor(UCommonInputSubsystem& InCommonInputSubsystem);
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override;

	float TryGetMouseWheelDelta(const float ResetThreshold) const;

protected:
	double PrevMouseWheelTimestamp{0.f};
	float PrevMouseWheelDelta{0.f};
};
