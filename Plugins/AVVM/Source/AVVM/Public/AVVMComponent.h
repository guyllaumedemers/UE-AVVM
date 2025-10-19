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
 *	UAVVMComponent is a system that defines presenter classes in BP and handle CRUD principle for the owned presenter objects. The component is expected to be pushed
 *	via GameFeature_AddComponent<T> and only target Client Actors.
 */
UCLASS(ClassGroup=("AVVM"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVM_API UAVVMComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
#if WITH_AUTOMATION_TESTS
	UAVVMPresenter* GetOrCreate(const TSubclassOf<UAVVMPresenter>& NewPresenter);
	void RemovePresenter(UAVVMPresenter* OldPresenter);
#endif

protected:
	// @gdemers A collection of Presenter Classes to be added to a given Actor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<TSubclassOf<UAVVMPresenter>> PresenterClasses;

	// @gdemers A collection of Presenters owned by the Component Outer.
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UAVVMPresenter>> TransientPresenters;
};
