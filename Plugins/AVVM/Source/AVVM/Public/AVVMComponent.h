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

#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"

#include "AVVMComponent.generated.h"

class UAVVMPresenter;

/**
 *	Class description:
 *
 *	UAVVMComponent is a runtime component added to Actor through GameFeature_AddComponent. Based on CRUD principle, it's job is to instance a UObject type (UPresenter)
 *	which bridge the "Gameplay code" and "UI code".
 */
UCLASS(ClassGroup=("AVVM"), meta=(BlueprintSpawnableComponent))
class AVVM_API UAVVMComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// @gdemers A collection of Presenter Classes to be added to a given Actor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UAVVMPresenter>> PresenterClasses;

	// @gdemers A collection of Presenters owned by the Component Outer.
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UAVVMPresenter>> TransientPresenters;
};
