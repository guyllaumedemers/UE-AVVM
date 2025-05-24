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

#include "DataRegistryId.h"
#include "UObject/Interface.h"

#include "AVVMResourceImplementer.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FKeepProcessingResources, const TArray<FDataRegistryId>&, QueuedResourcesId);

/**
 *	Class description:
 *
 *	UAVVMResourceImplementer interface that any Actor supporting the Resource Manager component have to implement
 *	to be able to support async loading of resources and forwarding loaded content to external systems.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMResourceImplementer : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMResourceImplementer
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	FDataRegistryId GetActorDefinitionResourceId() const;
	virtual FDataRegistryId GetActorDefinitionResourceId_Implementation() const PURE_VIRTUAL(GetActorDefinitionResourceId_Implementation, return FDataRegistryId(););

	UFUNCTION(BlueprintNativeEvent)
	bool CheckIsDoneAcquiringResources(const TArray<const UObject*>& Resource, FKeepProcessingResources Callback) const;
	virtual bool CheckIsDoneAcquiringResources_Implementation(const TArray<const UObject*>& Resource, FKeepProcessingResources Callback) const PURE_VIRTUAL(CheckIsDoneAcquiringResources_Implementation, return false;);
};
