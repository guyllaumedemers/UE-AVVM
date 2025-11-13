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
#include "AVVMAudioContext.h"

#include "AVVMAudioUtils.h"

UScriptStruct* TBaseStructure<FAVVMAudioContext>::Get()
{
	return FAVVMAudioContext::StaticStruct();
}

FAVVMSpatialAudioContext::FAVVMSpatialAudioContext(const AActor* NewInstigator,
                                                   const AActor* NewTarget,
                                                   const float NewWeight)
	: Instigator(NewInstigator)
	  , Target(NewTarget)
	  , Weight(NewWeight)
{
}

void FAVVMSpatialAudioContext::Notify(const UObject* WorldContextObject) const
{
	const AActor* Src = Instigator.Get();
	const AActor* Dest = Target.Get();

	if (!IsValid(Src) || !IsValid(Dest))
	{
		return;
	}

	const EAVVMSpacialDirection SpatialDirection = UAVVMAudioUtils::GetDirectionBetweenActors(Src, Dest);
	if (ensureAlwaysMsgf(SpatialDirection != EAVVMSpacialDirection::None,
	                     TEXT("DotProduct failed retrieving a valid Spatial Direction")))
	{
		// TODO @gdemers check if actor are in same room cluster
		// TODO @gdemers check if actor are in different separated by walls or object
		// TODO @gdemers check if actor are in different rooms but have visibility over each other
		const float SquaredDistance = Src->GetSquaredDistanceTo(Dest);
		QuerySpatialAudioSubsystem(WorldContextObject);
	}
}
