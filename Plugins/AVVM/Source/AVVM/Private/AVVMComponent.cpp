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
#include "AVVMComponent.h"

#include "AVVM.h"
#include "Archetypes/AVVMPresenter.h"
#include "GameFramework/Actor.h"

// @gdemers extern symbol for global access to custom LLM_tag
extern FLLMTagDeclaration LLMTagDeclaration_AVVMTag;

void UAVVMComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		return;
	}

	const FString ActorName = *Outer->GetName();
	UE_LOG(LogUI, Log, TEXT("Adding UAVVMComponent to Actor: %s"), *ActorName)
	TRACE_BOOKMARK(TEXT("Presenter.Create, TypedOuter: %s"), *ActorName);
	LLM_SCOPE_BYTAG(AVVMTag);

	// @gdemers presenters are created synchronously which makes it safe to broadcast events in the AActor::BeginPlay()
	// as all presenters will have registered with the notification subsystem.
	// just make sure, your derived AActor class doesn't execute code BEFORE the call to the super::BeginPlay().
	for (auto Iterator{PresenterClasses.CreateIterator()}; Iterator; ++Iterator)
	{
		if (IsValid(*Iterator))
		{
			auto* Presenter = NewObject<UAVVMPresenter>(this, Iterator->Get());
			TransientPresenters.AddUnique(Presenter);
		}
	}

	const UWorld* World = Outer->GetWorld();
	for (TObjectPtr<UAVVMPresenter>& Presenter : TransientPresenters)
	{
		Presenter->SafeBeginPlay(World);
	}
}

void UAVVMComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensure(IsValid(Outer)))
	{
		TransientPresenters.Empty();
		return;
	}

	const FString ActorName = Outer->GetName();
	UE_LOG(LogUI, Log, TEXT("Removing UAVVMComponent to Actor: %s"), *ActorName)
	TRACE_BOOKMARK(TEXT("Presenter.Destroy, TypedOuter: %s"), *ActorName);
	LLM_SCOPE_BYTAG(AVVMTag);

	for (TObjectPtr<UAVVMPresenter>& Presenter : TransientPresenters)
	{
		Presenter->SafeEndPlay();
	}

	TransientPresenters.Empty();
}

#if WITH_AUTOMATION_TESTS
UAVVMPresenter* UAVVMComponent::GetOrCreate(const TSubclassOf<UAVVMPresenter>& NewPresenterClass)
{
	if (TransientPresenters.IsEmpty())
	{
		auto* Presenter = NewObject<UAVVMPresenter>(GetTypedOuter<AActor>(), NewPresenterClass);
		TransientPresenters.Add(Presenter);
		return Presenter;
	}
	else
	{
		return TransientPresenters[0];
	}
}

void UAVVMComponent::RemovePresenter(UAVVMPresenter* OldPresenter)
{
	TransientPresenters.Remove(OldPresenter);
}
#endif
