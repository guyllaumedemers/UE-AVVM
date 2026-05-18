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

#include "InventoryProvider.h"
#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

#include "AutomatedTestInventoryActor.generated.h"

struct FAVVMActorIdentifierDataTableRow;
class UActorInventoryComponent;

/**
 *	Class description:
 *
 *	AAutomatedTestInventoryActor is an Actor class that impl the required interface to run functional test
 *	on the ActorInventoryComponent.
 */
UCLASS()
class INVENTORYSAMPLE_API AAutomatedTestInventoryActor : public AActor,
                                                         public IAVVMResourceProvider,
                                                         public IInventoryProvider
{
	GENERATED_BODY()

public:
	AAutomatedTestInventoryActor(const FObjectInitializer& ObjectInitializer);
	
	// IAVVMResourceProvider
	virtual int32 GetProviderUniqueId_Implementation() const override;
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const override;
	virtual TArray<FDataRegistryId> GetResourceDefinitionRegistryIds_Implementation() const override;
	virtual TArray<FDataRegistryId> CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const override;
	
	// IInventoryProvider
	virtual EItemSrcType GetItemSrcType_Implementation() const override;
	virtual bool DoesSupportSpawnOnLoad_Implementation() const override;

	void SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted);
	bool CheckContentIntegrity() const;
	bool CheckInventoryIntegrity() const;

	bool HasInventoryFinishedAllStreaming() const;
	void ForceCompletion() const;

	FOnResourceAsyncLoadingComplete GetOnCompleteDelegate();
	
	bool RunTest_ItemUniqueId(const TArray<const FAVVMActorIdentifierDataTableRow*>& ActorIdentifiers) const;
	bool RunTest_ItemStorageReference() const;
	bool RunTest_ItemStacking() const;

protected:
	UFUNCTION(CallInEditor)
	void OnRequestCompleted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMResourceManagerComponent> ResourceManagerComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UActorInventoryComponent> InventoryComponent = nullptr;

	// @gdemers exception. I never use mutable as it violate constness.
	UPROPERTY(Transient, BlueprintReadOnly)
	mutable int32 RandomProviderId = INDEX_NONE;

	TSharedPtr<bool> bIsAsyncProcessCompleted;
};
