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

#include "AutomatedTestSkillTreeValidationManager.generated.h"

class UActorSkillTreeComponent;

/**
 *	Class description:
 *	
 *	FInventoryValidationMechanism is a context struct that encapsulate information about the inventory system of a single entity.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeValidationMechanism
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ResourceProviderId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<int32> PrivateItemIds;
};

/**
 *	Class description:
 *	
 *	UAutomatedTestSkillTreeValidationManager is a class that run validation on resource loading/initialization process for the skill tree system
 *	during automated testing.
 */
UCLASS()
class SKILLSAMPLE_API UAutomatedTestSkillTreeValidationManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UFUNCTION(BlueprintCallable)
	static void Static_RegisterComponent(const UWorld* World,
	                                     const UActorSkillTreeComponent* SrcComponent);

	UFUNCTION(BlueprintCallable)
	static void Static_UnregisterComponent(const UWorld* World,
	                                       const UActorSkillTreeComponent* SrcComponent);

	UFUNCTION(BlueprintCallable)
	static void Static_PushPrivateItemId(const UWorld* World,
	                                     const UActorSkillTreeComponent* SrcComponent,
	                                     const int32 NewPrivateItemId);

protected:
	static UAutomatedTestSkillTreeValidationManager* Get(const UWorld* World);
	void RegisterComponent(const UActorSkillTreeComponent* SrcComponent);
	void UnregisterComponent(const UActorSkillTreeComponent* SrcComponent);
	void PushPrivateItemId(const UActorSkillTreeComponent* SrcComponent, const int32 NewPrivateItemId);

	UFUNCTION(CallInEditor)
	void OnPlayerStateUniqueNetIdReady(const FUniqueNetIdRepl& UniqueNetIdRepl,
	                                   const UActorSkillTreeComponent* SrcComponent);

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const UActorSkillTreeComponent>, FSkillTreeValidationMechanism> ValidationMechanisms;
};
