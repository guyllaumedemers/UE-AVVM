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

#include "AVVMDoesActorSupportInstanceIdentifier.generated.h"

/**
 *	Class description:
 *	
 *	IAVVMDoesActorSupportInstanceIdentifier is an interface to be impl in an actor class that expect
 *	access to instance specific identifier.
 *	
 *	See : FAVVMSocketTargetingHelper::Static_AttachToActorAsync & FAVVMSocketTargetingHelper::GetDesiredTypedInner
 *	to understand how this identifier is used.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMDoesActorSupportInstanceIdentifier : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMDoesActorSupportInstanceIdentifier
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int32 GetInstancedId() const;
	virtual int32 GetInstancedId_Implementation() const;

	UFUNCTION(BlueprintNativeEvent)
	void SetInstancedId(const int32 NewInstancedId);
	virtual void SetInstancedId_Implementation(const int32 NewInstancedId);
	
private:
	// @gdemers actor that require dynamic socketing need to be identifiable via ID.
	int32 InstancedId{INDEX_NONE};
};
