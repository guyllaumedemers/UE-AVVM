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

#include "UObject/Object.h"

#include "HandshakeValidatorImpl.generated.h"

struct FHandshakeContext;

/**
 *	Class description:
 *
 *	UHandshakeValidatorImpl is an object that implement the 'strategy pattern' and allow behaviour changes by
 *	referencing a different class context at runtime. It validates the 'Handshake' between two components of similar types.
 */
UCLASS()
class HANDSHAKESAMPLE_API UHandshakeValidatorImpl : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void TryExecute(const FHandshakeContext& Context) const;

protected:
	virtual bool DoesMeetRequirementForHandshake(const FHandshakeContext& Context) const;
	virtual void TryExecute_Internal(const FHandshakeContext& Context) const;
};
