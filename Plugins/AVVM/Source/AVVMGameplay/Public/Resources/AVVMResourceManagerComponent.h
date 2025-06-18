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
#include "DataRegistryTypes.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"

#include "AVVMResourceManagerComponent.generated.h"

/**
 *	Class description:
 *
 *	UAVVMResourceManagerComponent dynamic component handling resource loading on any Actor that implement the required interface
 *	IAVVMResourceImplementer.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMResourceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	void RequestExternalResourceAsync(const AActor* Outer);
	void OnRegistryIdAcquired(const FDataRegistryAcquireResult& Result);
	void OnSoftObjectAcquired();

	UFUNCTION()
	bool OnProcessAdditionalResources(const TArray<FDataRegistryId>& PendingRegistriesId);

	TArray<TSharedPtr<FStreamableHandle>> ResourceHandles;
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
