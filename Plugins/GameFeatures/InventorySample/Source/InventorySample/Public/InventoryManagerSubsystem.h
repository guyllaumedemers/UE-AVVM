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

#include "Subsystems/WorldSubsystem.h"

#include "InventoryManagerSubsystem.generated.h"

/**
*	Class description:
 *
 *	UInventoryManagerSubsystem is a subsystem instancing ItemActors in World through project specific
 *	implementation details.
 *
 *	It represents the intermediate entity that execute actions between ActorInventoryComponents, and handle external request
 *	for action such as : trades, slot swapping, or equipping, etc...
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	UFUNCTION(BlueprintCallable)
	static UInventoryManagerSubsystem* GetSubsystem(const UWorld* World);

	AActor* CreateItemActor(const UClass* ItemActorClass,
	                        const FActorSpawnParameters& SpawnParams) const;

	// TODO @gdemers add support for the above mentioned interactions between two Authoritative ActorInventoryComponents.

protected:
	// @gdemers factory method to support pooling or other instancing system specific to your project.
	virtual AActor* Factory(const UClass* ItemActorClass, const FActorSpawnParameters& SpawnParams) const;
	// @gdemers shutdown method to support pooling or other instancing system specific to your project.
	virtual void Shutdown(AActor* ItemActor);
};
