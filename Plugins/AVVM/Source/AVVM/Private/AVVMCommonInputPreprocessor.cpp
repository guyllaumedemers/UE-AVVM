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
#include "AVVMCommonInputPreprocessor.h"

#include "AVVMToolkitUtils.h"
#include "Engine/Engine.h"

const FAVVMCommonInputPreprocessor* UAVVMCommonInputSubsystem::Static_GetMouseProcessor(const ULocalPlayer* LocalPlayer)
{
	const auto* Subsystem = Cast<UAVVMCommonInputSubsystem>(UCommonInputSubsystem::Get(LocalPlayer));
	return IsValid(Subsystem) ? Subsystem->GetMouseProcessor() : nullptr;
}

float UAVVMCommonInputSubsystem::Static_GetMouseWheelDelta(const ULocalPlayer* LocalPlayer)
{
	const auto* Subsystem = Cast<UAVVMCommonInputSubsystem>(UCommonInputSubsystem::Get(LocalPlayer));
	return IsValid(Subsystem) ? Subsystem->GetMouseWheelDelta() : 0.f;
}

TSharedPtr<FCommonInputPreprocessor> UAVVMCommonInputSubsystem::MakeInputProcessor()
{
	return MakeShared<FAVVMCommonInputPreprocessor>(*this);
}

const FAVVMCommonInputPreprocessor* UAVVMCommonInputSubsystem::GetMouseProcessor() const
{
	ensureAlwaysMsgf(CommonInputPreprocessor.IsValid(), TEXT("Attempting invalid Mouse processor access."));
	return StaticCast<FAVVMCommonInputPreprocessor*>(CommonInputPreprocessor.Get());
}

float UAVVMCommonInputSubsystem::GetMouseWheelDelta() const
{
	const auto* InputPreprocessor = GetMouseProcessor();
	if (ensureAlwaysMsgf(InputPreprocessor != nullptr, TEXT("Invalid Cast.")))
	{
		return InputPreprocessor->TryGetMouseWheelDelta(InputFlushThreshold);
	}
	else
	{
		return 0.f;
	}
}

FAVVMCommonInputPreprocessor::FAVVMCommonInputPreprocessor(UCommonInputSubsystem& InCommonInputSubsystem)
	: FCommonInputPreprocessor(InCommonInputSubsystem)
{
}

bool FAVVMCommonInputPreprocessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp,
                                                                  const FPointerEvent& InWheelEvent,
                                                                  const FPointerEvent* InGestureEvent)
{
	const bool bResult = FCommonInputPreprocessor::HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGestureEvent);
	if (!bResult)
	{
		return false;
	}

	// @gdemers we need to be able to hijack mouse events to cache relevant information outside the fire-n-forget events
	// received from the operating system. Note : I havent found within the FSlateApplication class relevant property caching wheel delta information, hence
	// why this override exist. 
	const double Now = UAVVMToolkitUtils::GetServerWorldTime(GEngine);
	PrevMouseWheelTimestamp = Now;
	PrevMouseWheelDelta = InWheelEvent.GetWheelDelta();

	return true;
}

float FAVVMCommonInputPreprocessor::TryGetMouseWheelDelta(const float ResetThreshold) const
{
	// @gdemers safeguard surrounding Mouse Wheel delta access. If ever an external resource trigger access to this information
	// outside an input event, we can mitigate the effect outcome by invalidating the value returned.
	const double Now = UAVVMToolkitUtils::GetServerWorldTime(GEngine);
	const bool bShouldIgnoreGetRequest = ((Now - PrevMouseWheelTimestamp) > ResetThreshold);
	return bShouldIgnoreGetRequest ? PrevMouseWheelDelta : 0.f;
}
