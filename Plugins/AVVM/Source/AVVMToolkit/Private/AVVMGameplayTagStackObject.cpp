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

#include "AVVMGameplayTagStackObject.h"

#include "AVVMLogger.h"
#include "AVVMToolkitModule.h"
#include "Engine/Engine.h"

FAVVMGameplayTagStackObject::FAVVMGameplayTagStackObject(const FGameplayTag& NewTag,
                                                         const int32 NewReplicatedStackCount,
                                                         const int32 NewPredictedStackCount)
	: Tag(NewTag),
	  ReplicatedStackCount(NewReplicatedStackCount),
	  PredictedStackCount(NewPredictedStackCount)
{
}

bool FAVVMGameplayTagStackObject::operator==(const FAVVMGameplayTagStackObject& Rhs) const
{
	return (Tag == Rhs.Tag) &&
			(ReplicatedStackCount == Rhs.ReplicatedStackCount) &&
			(PredictedStackCount == Rhs.PredictedStackCount);
}

void FAVVMGameplayTagStackObject::PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("New FAVVMGameplayTagStackObject::%s Detected!"),
	                *Tag.ToString());

	auto* FastArraySerializer = StaticCast<FAVVMGameplayTagStackObjectFastArray*>(const_cast<FFastArraySerializer*>(&InArraySerializer));
	if (ensureAlwaysMsgf(FastArraySerializer != nullptr, TEXT("Failed to cast to owning type.")))
	{
		auto*& OutValue = FastArraySerializer->FastLookup.FindOrAdd(Tag);
		OutValue = this;

		PredictedStackCount = ReplicatedStackCount;
		FastArraySerializer->OnGameplayTagStackObjectChanged.Broadcast(Tag, PredictedStackCount);
	}
}

void FAVVMGameplayTagStackObject::PreReplicatedRemove(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("Removing FAVVMGameplayTagStackObject::%s Detected!"),
	                *Tag.ToString());

	auto* FastArraySerializer = StaticCast<FAVVMGameplayTagStackObjectFastArray*>(const_cast<FFastArraySerializer*>(&InArraySerializer));
	if (ensureAlwaysMsgf(FastArraySerializer != nullptr, TEXT("Failed to cast to owning type.")) && FastArraySerializer->FastLookup.Contains(Tag))
	{
		FastArraySerializer->FastLookup.Remove(Tag);
		FastArraySerializer->OnGameplayTagStackObjectRemoved.Broadcast(Tag);
	}
}

void FAVVMGameplayTagStackObject::PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("FAVVMGameplayTagStackObject::%s Value Changed Detected. %d!"),
	                *Tag.ToString(),
	                ReplicatedStackCount);

	auto* FastArraySerializer = StaticCast<FAVVMGameplayTagStackObjectFastArray*>(const_cast<FFastArraySerializer*>(&InArraySerializer));
	if (ensureAlwaysMsgf(FastArraySerializer != nullptr, TEXT("Failed to cast to owning type.")))
	{
		PredictedStackCount = ReplicatedStackCount;
		FastArraySerializer->OnGameplayTagStackObjectChanged.Broadcast(Tag, PredictedStackCount);
	}
}

int32 FAVVMGameplayTagStackObjectFastArray::Modify(const FGameplayTag& Tag,
                                                   const int32 DeltaValue,
                                                   const bool bHasAuthority)
{
	FAVVMGameplayTagStackObject** SearchResult = FastLookup.Find(Tag);
	if ((SearchResult == nullptr) && bHasAuthority)
	{
		MarkArrayDirty();

		auto& Value = GameplayTagStackObjects.Add_GetRef(FAVVMGameplayTagStackObject{Tag, DeltaValue, DeltaValue});
		FastLookup.Add(Tag, &Value);
	}
	else if (SearchResult == nullptr)
	{
		AVVM_LOGGER_LOG(LogToolkit,
		                GEngine,
		                GEngine,
		                TEXT("Attempting to access %s from Client before initial FAVVMGameplayTagStackObject replication."),
		                *Tag.ToString());
	}
	else if ((*SearchResult) != nullptr)
	{
		if (bHasAuthority)
		{
			MarkItemDirty(**SearchResult);

			(*SearchResult)->ReplicatedStackCount = FMath::Clamp((*SearchResult)->ReplicatedStackCount + DeltaValue, 0, INT32_MAX);
			return (*SearchResult)->ReplicatedStackCount;
		}
		else
		{
			(*SearchResult)->PredictedStackCount = FMath::Clamp((*SearchResult)->PredictedStackCount + DeltaValue, 0, INT32_MAX);
			return (*SearchResult)->PredictedStackCount;
		}
	}

	return DeltaValue;
}

bool FAVVMGameplayTagStackObjectFastArray::operator==(const FAVVMGameplayTagStackObjectFastArray& Rhs) const
{
	return (GameplayTagStackObjects == Rhs.GameplayTagStackObjects) &&
			FastLookup.OrderIndependentCompareEqual(Rhs.FastLookup);
}

const TArray<FAVVMGameplayTagStackObject>& FAVVMGameplayTagStackObjectFastArray::GetGameplayTagStackObjects() const
{
	return GameplayTagStackObjects;
}

const FAVVMGameplayTagStackObject* FAVVMGameplayTagStackObjectFastArray::GetFastLookupTag(const FGameplayTag& Tag) const
{
	return FastLookup.Contains(Tag) ? FastLookup[Tag] : nullptr;
}
