﻿//Copyright(c) 2025 gdemers
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

#include "AVVMResourceProvider.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FKeepProcessingResources, const TArray<FDataRegistryId>&, QueuedResourcesId);

/**
 *	Class description:
 *
 *	UAVVMResourceProvider provide Actor resource definition via Registry id and support runtime checking of async loaded resources.
 *	It interface with the UAVVMResourceManagerComponent during the loading phase of the Resource Manager Component and forward loaded content to external systems.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMResourceProvider : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMResourceProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FDataRegistryId GetActorDefinitionResourceId() const;
	virtual FDataRegistryId GetActorDefinitionResourceId_Implementation() const PURE_VIRTUAL(GetActorDefinitionResourceId_Implementation, return FDataRegistryId(););

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CheckIsDoneAcquiringResources(const TArray<UObject*>& Resources,
	                                   const FKeepProcessingResources& Callback) const;

	virtual bool CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources,
	                                                          const FKeepProcessingResources& Callback) const PURE_VIRTUAL(CheckIsDoneAcquiringResources_Implementation, return false;);
};

/**
 *	Class description:
 *
 *	UAVVMResourceHandlingBlueprintFunctionLibrary define a set of api to allow reusability and circumvent some issues that arise
 *	with design decision and how BP prevent Dynamic Delegates from being broadcast from BP code.
 */
UCLASS(BlueprintType)
class AVVMGAMEPLAY_API UAVVMResourceHandlingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static TArray<FDataRegistryId> CheckAbilities(UAbilitySystemComponent* AbilitySystemComponent,
	                                              const TArray<UObject*>& Resources);

	UFUNCTION(BlueprintCallable)
	static bool ExecuteResourceProviderDelegate(const TArray<FDataRegistryId>& QueuedResourcesId,
	                                            const FKeepProcessingResources& Callback);
};
