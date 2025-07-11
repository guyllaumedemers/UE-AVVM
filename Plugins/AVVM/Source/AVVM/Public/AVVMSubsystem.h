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

#include "AVVMSubsystem.generated.h"

class UAVVMPresenter;
class UMVVMViewModelBase;

/**
 *	Class description:
 *
 *	FAVVMPresenterContextArgs encapsulate arguments of UAVVMSubsystem api for better code readability.
 */
USTRUCT(BlueprintType)
struct AVVM_API FAVVMPresenterContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UWorld> WorldContext = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UAVVMPresenter> Presenter = nullptr;
};

/**
 *	Class description:
 *
 *	UAVVMSubsystem is based on CRUD principle. It Create/Read/Update/Destroy "Manual" ViewModel type with the
 *	using UMVVM plugin api.
 */
UCLASS(ClassGroup=("AVVM"))
class AVVM_API UAVVMSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UAVVMSubsystem* Get(const UWorld* WorldContext);

	static bool Static_UnregisterPresenter(const FAVVMPresenterContextArgs& Context);
	static UMVVMViewModelBase* Static_RegisterPresenter(const FAVVMPresenterContextArgs& Context);

protected:
	struct FAVVMViewModelKVP
	{
		~FAVVMViewModelKVP();

		UMVVMViewModelBase* GetOrCreate(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass,
		                                UObject* Outer);

		bool RemoveOrDestroy(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass);

	private:
		// @gdemers A given Actor can be referenced by multiple UAVVMPresenter and a ViewModel instance may have to be rebound
		// to a View, reusing already created View Model class.
		TMap<const TSubclassOf<UMVVMViewModelBase>, TStrongObjectPtr<UMVVMViewModelBase>> ViewModelClassToViewModelInstance;

		// @gdemers RefCount target the number of user of the Actor.
		uint32 RefCounter = 0;
	};

	UMVVMViewModelBase* GetOrCreate(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass,
	                                AActor* Outer);

	bool RemoveOrDestroy(const TSubclassOf<UMVVMViewModelBase>& ViewModelClass,
	                     AActor* Outer);

	// @gdemers A collection of unique Actors to a set of ViewModel bound by the TypedOuter<AActor>().
	// TWeakObjectPtr<AACtor> will remain valid throughout the PIE session as the AActor referenced is the TypedOuter.
	TMap<TWeakObjectPtr<const AActor>, FAVVMViewModelKVP> ActorToViewModelCollection;
};
