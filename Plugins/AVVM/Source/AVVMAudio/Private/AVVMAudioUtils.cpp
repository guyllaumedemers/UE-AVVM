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
#include "AVVMAudioUtils.h"

#include "AVVMAudioContext.h"

void UAVVMAudioUtils::NotifyAudioCue(const UObject* WorldContextObject, const TInstancedStruct<FAVVMAudioContext>& Context)
{
	const auto* NotificationContext = Context.GetPtr<FAVVMAudioContext>();
	if (ensureAlwaysMsgf(NotificationContext != nullptr,
	                     TEXT("FAVVMAudioContext is invalid!")))
	{
		NotificationContext->Notify(WorldContextObject);
	}
}

EAVVMSpacialDirection UAVVMAudioUtils::GetDirectionBetweenActors(const AActor* Actor, const AActor* OtherActor)
{
	if (!IsValid(Actor) || !IsValid(OtherActor))
	{
		return EAVVMSpacialDirection::None;
	}

	const float DotProduct = Actor->GetDotProductTo(OtherActor);
	if (FMath::IsNearlyZero(DotProduct))
	{
		return EAVVMSpacialDirection::Perpendicular;
	}
	else if (DotProduct > 0.f)
	{
		return EAVVMSpacialDirection::SameDirection;
	}
	else if (DotProduct < 0.f && !FMath::IsNearlyEqual(DotProduct, -2.f/*GetDotProductTo Failure Output*/))
	{
		return EAVVMSpacialDirection::OppositeDirection;
	}
	else
	{
		return EAVVMSpacialDirection::None;
	}
}
