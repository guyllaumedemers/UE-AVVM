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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"

#include "InteractionComponent.generated.h"

/**
 *	Class description:
 *
 *	UInteractionComponent handle user interaction with world objects. We expect the GFP to _AddComponent to Actors defined in the Project
 *	and to be available ONLY the client side.
 *
 *	OnBeginOverlap/OnEndOverlap locally add loose tag for the ability system to execute behaviour based on locally controlled pawn via
 *	Input Pressed/Released.
 *
 *	Note : Im sceptical about converting this system to use an ability system component and custom AbilityTask_WaitforBeginOverlap/AbilityTask_WaitforEndOverlap,
 *	the task overlap would end whenever capturing an overlap and push a blocking tag. it counter part task would wait for the end overlap
 *	and remove the tag. Doing so would prevent second interaction from happening, if desired! A second player exiting the overlap would however trigger the EndOverlap.
 *
 *	The sequence would also have to be repeated... How would dormancy work with those ? Can Ability Task become dormant and be woken up after ?
 */
UCLASS(ClassGroup=("Interaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INTERACTIONSAMPLE_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION()
	void OnPrimitiveComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	                                      AActor* OtherActor,
	                                      UPrimitiveComponent* OtherComp,
	                                      int32 OtherBodyIndex,
	                                      bool bFromSweep,
	                                      const FHitResult& SweepResult);

	UFUNCTION()
	void OnPrimitiveComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                                    AActor* OtherActor,
	                                    UPrimitiveComponent* OtherComp,
	                                    int32 OtherBodyIndex);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldPreventContingency = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GrantAbilityTag = FGameplayTag::EmptyTag;

	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
