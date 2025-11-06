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

#include "Batchable.generated.h"

/**
 *	Class description:
 *
 *	UBatchable is an Interface Class that binds a user to a batching context. This allow
 *	removal behaviour if ever the batchable implementer require removal from the context
 *	due to runtime events that occur.
 *
 *	example : we have a collection of ammo on the ground for pick ups. Nobody ever pick them.
 *	We attempt batch destroying them. A player pass-by and pick up, before the process complete
 *	and destroy the batch.
 */
UINTERFACE(BlueprintType)
class BATCHSAMPLE_API UBatchable : public UInterface
{
	GENERATED_BODY()
};

class BATCHSAMPLE_API IBatchable
{
	GENERATED_BODY()

public:
	void SetOwningBatchIndex(const int32 Index);
	int32 GetOwningBatchIndex() const;
	bool HasValidBatchIndex() const;
	bool IsPlacedInEditor() const;

private:
	// @demers can override but never call in derived type. good example of proper encapsulation.
	virtual const AActor* GetSelf() const PURE_VIRTUAL(GetSelf, return nullptr;);
	int32 BatchIndex = INDEX_NONE;
};
