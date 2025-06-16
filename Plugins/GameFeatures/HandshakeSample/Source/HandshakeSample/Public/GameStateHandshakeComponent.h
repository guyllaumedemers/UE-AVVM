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

#include "GameStateHandshakeComponent.generated.h"

class UActorComponent;
class UHandshakeValidatorImpl;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHandshakeComplete, const bool, bWasSuccess);

/**
 *	Class description:
 *
 *	FHandshakeContext define the endpoint of a transaction between two entities.
 */
USTRUCT(BlueprintType)
struct HANDSHAKESAMPLE_API FHandshakeContext
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	UActorComponent* Src = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	UActorComponent* Dest = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	FOnHandshakeComplete CompletionDelegate;
};

/**
 *	Class description:
 *
 *	UGameStateHandshakeComponent handle execution of a handshake based on ability invocation. Both end of an Ability, Instigator and Target often require
 *	a set of requirements to be met, validation can be run through the Impl object provided, which make this system flexible for extension, and handle hiding the details
 *	of an exchange between both entity.
 */
UCLASS(ClassGroup=("Handshake"), Blueprintable, meta=(BlueprintSpawnableComponent))
class HANDSHAKESAMPLE_API UGameStateHandshakeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UGameStateHandshakeComponent* GetActorComponent(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void TryExecuteHandshake(const UHandshakeValidatorImpl* HandshakeImpl,
	                         const FHandshakeContext& Context) const;
};
