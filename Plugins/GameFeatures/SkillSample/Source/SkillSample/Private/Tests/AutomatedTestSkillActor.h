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

#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

#include "AutomatedTestSkillActor.generated.h"

struct FAVVMActorIdentifierDataTableRow;
class UActorSkillTreeComponent;

/**
 *	Class description:
 *
 *	AAutomatedTestSkillActor is an Actor class that impl the required interface to run functional test
 *	on the ActorSkillTreeComponent.
 */
UCLASS()
class SKILLSAMPLE_API AAutomatedTestSkillActor : public AActor,
                                                 public IAVVMResourceProvider
{
	GENERATED_BODY()

public:
	AAutomatedTestSkillActor(const FObjectInitializer& ObjectInitializer);

	// IAVVMResourceProvider
	virtual int32 GetProviderUniqueId_Implementation() const override;
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const override;
	virtual TArray<FDataRegistryId> GetResourceDefinitionRegistryIds_Implementation() const override;
	virtual TArray<FDataRegistryId> CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const override;

	void SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted);
	bool CheckContentIntegrity() const;
	bool CheckSkillTreeIntegrity() const;
	
	bool HasSkillTreeFinishedAllStreaming() const;
	void ForceCompletion() const;

	FOnResourceAsyncLoadingComplete GetOnCompleteDelegate();

protected:
	UFUNCTION(CallInEditor)
	void OnRequestCompleted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAVVMResourceManagerComponent> ResourceManagerComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UActorSkillTreeComponent> SkillTreeComponent = nullptr;

	// @gdemers exception. I never use mutable as it violate constness.
	UPROPERTY(Transient, BlueprintReadOnly)
	mutable int32 RandomProviderId = INDEX_NONE;

	TSharedPtr<bool> bIsAsyncProcessCompleted;
};
