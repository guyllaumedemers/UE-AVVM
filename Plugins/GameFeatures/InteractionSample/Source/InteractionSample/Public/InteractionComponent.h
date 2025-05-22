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
 *	EInteractionStatusType define a set of possible status the component can be in.
 */
UENUM(BlueprintType)
enum class EInteractionStatusType : uint8
{
	None,
	Blocked,
	Active,
	Interrupted,
	Completed
};

inline const TCHAR* EnumToString(EInteractionStatusType State)
{
	switch (State)
	{
		case EInteractionStatusType::Blocked:
			return TEXT("Blocked");
		case EInteractionStatusType::Active:
			return TEXT("Active");
		case EInteractionStatusType::Interrupted:
			return TEXT("Interrupted");
		case EInteractionStatusType::Completed:
			return TEXT("Completed");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	UInteractionComponent handle user interaction with world objects. We expect the GFP to _AddComponent to Actors defined in the Project
 *	and to be available ONLY the client side.
 *
 *	OnBeginOverlap/OnEndOverlap locally add loose tag for the ability system to execute behaviour based on locally controlled pawn via
 *	Input Pressed/Released.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
	FGameplayTag EventTag = FGameplayTag::EmptyTag;

	TWeakObjectPtr<const AActor> WorldActor = nullptr;
};
