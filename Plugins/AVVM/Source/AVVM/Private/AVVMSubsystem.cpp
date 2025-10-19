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
#include "AVVMSubsystem.h"

#include "AVVM.h"
#include "MVVMViewModelBase.h"
#include "Archetypes/AVVMPresenter.h"
#include "Engine/World.h"

bool UAVVMSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (IsValid(World))
	{
		return !World->IsNetMode(NM_DedicatedServer);
	}

	return false;
}

void UAVVMSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogUI, Log, TEXT("UAVVMSubsystem::Initialize. Running On Client."));
}

void UAVVMSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ActorToViewModelCollection.Empty();
}

UAVVMSubsystem* UAVVMSubsystem::Get(const UWorld* WorldContext)
{
	return UWorld::GetSubsystem<UAVVMSubsystem>(WorldContext);
}

bool UAVVMSubsystem::Static_UnregisterPresenter(const FAVVMPresenterContextArgs& Context)
{
	auto* AVVMSubsystem = UAVVMSubsystem::Get(Context.WorldContext.Get());
	if (IsValid(AVVMSubsystem))
	{
		const UAVVMPresenter* Presenter = Context.Presenter.Get();
		const TSubclassOf<UMVVMViewModelBase> ViewModelClass = Presenter->GetViewModelClass();
		AActor* OuterKey = Presenter->GetOuterKey();
		return IsValid(ViewModelClass) ? AVVMSubsystem->RemoveOrDestroy(ViewModelClass, OuterKey) : false;
	}

	return false;
}

UMVVMViewModelBase* UAVVMSubsystem::Static_RegisterPresenter(const FAVVMPresenterContextArgs& Context)
{
	auto* AVVMSubsystem = UAVVMSubsystem::Get(Context.WorldContext.Get());
	if (IsValid(AVVMSubsystem))
	{
		const UAVVMPresenter* Presenter = Context.Presenter.Get();
		const TSubclassOf<UMVVMViewModelBase> ViewModelClass = Presenter->GetViewModelClass();
		AActor* OuterKey = Presenter->GetOuterKey();
		return IsValid(ViewModelClass) ? AVVMSubsystem->GetOrCreate(ViewModelClass, OuterKey) : nullptr;
	}

	return nullptr;
}

#if WITH_AUTOMATION_TESTS
int32 UAVVMSubsystem::Static_GetPresentersCount(const UWorld* World)
{
	auto* AVVMSubsystem = UAVVMSubsystem::Get(World);
	if (IsValid(AVVMSubsystem))
	{
		int32 Count = 0;
		for (const auto& [Actor, ViewModelKVP] : AVVMSubsystem->ActorToViewModelCollection)
		{
			Count += ViewModelKVP.GetPresenterCount();
		}

		return Count;
	}

	return INDEX_NONE;
}

int32 UAVVMSubsystem::Static_GetActorCount(const UWorld* World)
{
	auto* AVVMSubsystem = UAVVMSubsystem::Get(World);
	if (IsValid(AVVMSubsystem))
	{
		return AVVMSubsystem->ActorToViewModelCollection.Num();
	}

	return INDEX_NONE;
}
#endif

UAVVMSubsystem::FAVVMViewModelKVP::~FAVVMViewModelKVP()
{
	ViewModelClassToViewModelInstance.Empty();
}

UMVVMViewModelBase* UAVVMSubsystem::FAVVMViewModelKVP::GetOrCreate(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass,
                                                                   UObject* Outer)
{
	if (!ViewModelClassToViewModelInstance.Contains(ViewModelClass))
	{
		TStrongObjectPtr<UMVVMViewModelBase>& SearchResult = ViewModelClassToViewModelInstance.Add(ViewModelClass);
		SearchResult = TStrongObjectPtr(NewObject<UMVVMViewModelBase>(Outer, ViewModelClass.Get()));
		++RefCounter;
		return SearchResult.Get();
	}
	else
	{
		return ViewModelClassToViewModelInstance[ViewModelClass].Get();
	}
}

bool UAVVMSubsystem::FAVVMViewModelKVP::RemoveOrDestroy(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass)
{
	if (ViewModelClassToViewModelInstance.Contains(ViewModelClass))
	{
		ViewModelClassToViewModelInstance.Remove(ViewModelClass);
		--RefCounter;
	}

	return (false == !!RefCounter);
}

#if WITH_AUTOMATION_TESTS
int32 UAVVMSubsystem::FAVVMViewModelKVP::GetPresenterCount() const
{
	return ViewModelClassToViewModelInstance.Num();
}
#endif

UMVVMViewModelBase* UAVVMSubsystem::GetOrCreate(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass,
                                                AActor* Outer)
{
	const TWeakObjectPtr<AActor> WeakObjectPtr = MakeWeakObjectPtr<AActor>(Outer);
	FAVVMViewModelKVP& SearchResult = ActorToViewModelCollection.FindOrAdd(WeakObjectPtr);
	return SearchResult.GetOrCreate(ViewModelClass, this);
}

bool UAVVMSubsystem::RemoveOrDestroy(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass,
                                     AActor* Outer)
{
	const TWeakObjectPtr<AActor> WeakObjectPtr = MakeWeakObjectPtr<AActor>(Outer);
	FAVVMViewModelKVP& SearchResult = ActorToViewModelCollection.FindOrAdd(WeakObjectPtr);
	const bool bIsEmpty = SearchResult.RemoveOrDestroy(ViewModelClass);
	if (bIsEmpty)
	{
		ActorToViewModelCollection.Remove(WeakObjectPtr);
	}

	return bIsEmpty;
}
