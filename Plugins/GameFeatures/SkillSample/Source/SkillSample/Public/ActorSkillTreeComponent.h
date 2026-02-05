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

#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"

#include "ActorSkillTreeComponent.generated.h"

class USkillTreeNodeObject;

/**
 *	Class description:
 *	
 *	UActorSkillTreeComponent is a component object that handle granting modifiers based on a list of {FDataRegistryId} provided by its owning outer
 *	following a backend request, or Data Asset referencing. Progression is expected to be supported for USkillTreeNodeObject upgrades!
 */
UCLASS()
class SKILLSAMPLE_API UActorSkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorSkillTreeComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UActorSkillTreeComponent* GetActorComponent(const AActor* NewActor);
	
protected:
	// @gdemers : set of GE granted from CDO objects referenced based on backend or data asset
	// retrieval.
	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<TObjectPtr<USkillTreeNodeObject>/*SkillTree Node derived CDO*/, FActiveGameplayEffectHandle> GameplayEffectHandles;
};
