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

#include "BatchingRule.h"

bool UBatchingRule::DoesQualifyForBatchDestroy(const AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const UClass* InspectClass = Actor->GetClass();
	if (!IsValid(InspectClass))
	{
		return false;
	}

	if (!bAllowBatchDestroyChildClasses)
	{
		const bool bIsIgnored = IgnoredClasses.Contains(InspectClass);
		if (bIsIgnored)
		{
			return false;
		}

		return Classes.Contains(InspectClass);
	}
	else
	{
		bool HasChild = false;

		for (const TSubclassOf<AActor>& ActorClass : IgnoredClasses)
		{
			HasChild |= InspectClass->IsChildOf(ActorClass);
		}

		if (HasChild)
		{
			return false;
		}

		for (const TSubclassOf<AActor>& ActorClass : Classes)
		{
			HasChild |= InspectClass->IsChildOf(ActorClass);
		}

		return HasChild;
	}
}

int32 UBatchingRule::GetMaxSizePerBatchDestroy() const
{
	return MaxSizePerBatchDestroy;
}

float UBatchingRule::GetBatchInterval() const
{
	return IntervalBetweenBatchDestroy;
}

float UBatchingRule::GetMaxLifetimeAllowedToUndersizeBatch() const
{
	return MaxLifetimeAllowedToUndersizeBatch;
}

bool UBatchingRule::IsBatchDestroyEnabled() const
{
	return bEnableBatchDestroy;
}
