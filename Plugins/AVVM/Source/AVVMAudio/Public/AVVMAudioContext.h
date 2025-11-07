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

#include "StructUtils/InstancedStruct.h"

#include "AVVMAudioContext.generated.h"

class AActor;

/**
 *	Class description:
 *
 *	FAVVMAudioContext is a context struct that define the parameters required to execute an audio cue, and
 *	notify its implementation specific audio system. 
 */
USTRUCT(BlueprintType)
struct AVVMAUDIO_API FAVVMAudioContext
{
	GENERATED_BODY()

	virtual ~FAVVMAudioContext() = default;
	virtual void Notify(const UObject* WorldContextObject) const PURE_VIRTUAL(Notify, return;);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FAVVMAudioContextArgs>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FAVVMAudioContext> Make(TArgs&&... Args);
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FAVVMAudioContext> FAVVMAudioContext::Make(TArgs&&... Args)
{
	return TInstancedStruct<FAVVMAudioContext>::Make<TChild>(Forward<TArgs>(Args)...);
}

/**
 *	Class description:
 *
 *	
 */
USTRUCT(BlueprintType)
struct AVVMAUDIO_API FAVVMSpatialAudioContext : public FAVVMAudioContext
{
	GENERATED_BODY()

	FAVVMSpatialAudioContext() = default;
	explicit FAVVMSpatialAudioContext(const AActor* NewInstigator,
	                                  const AActor* NewTarget,
	                                  const float NewWeight);
	
	virtual void Notify(const UObject* WorldContextObject) const override;

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const AActor> Instigator = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const AActor> Target = nullptr;

	// @gdemers could be the dot product result to provide directional weight
	UPROPERTY(Transient, BlueprintReadWrite, meta=(ToolTip="Weight value that can modify audio cue intensity based on gameplay context."))
	float Weight = 0.f;

private:
	// @gdemers DONT change protection level! private virtual call can be overriden in derive type but not invoked which
	// is exactly what we want here.
	virtual void QuerySpatialAudioSubsystem(const UObject* WorldContextObject) const PURE_VIRTUAL(QuerySpatialAudioSubsystem, return;);
};
