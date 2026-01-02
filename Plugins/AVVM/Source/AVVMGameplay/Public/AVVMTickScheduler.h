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

#include "AVVMTickScheduler.generated.h"

class IAVVMDoesSupportManualTicking;
class UAVVMTickSchedulerRule;

/**
 *	Class description:
 *	
 *	UAVVMTickScheduler is a subsystem that manage Tick aggregation for AActor/UActorComponent who
 *	perform badly on the game thread due to frequent/and sparse memory access/load of the polymorphic Tick function.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMTickScheduler : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	static void Static_Register(const UWorld* World,
	                            const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTickActor);

	UFUNCTION(BlueprintCallable)
	static void Static_UnRegister(const UWorld* World,
	                              const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTickActor);
	
#if !UE_BUILD_SHIPPING
	void OnTickSchedulerRuleCVarChanged();
#endif

protected:
	static UAVVMTickScheduler* Get(const UWorld* World);
	void Register(const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTicker);
	void UnRegister(const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTicker);
	void GetSetProjectTickSchedulerRule();
	void InitRule();

	// @gdemers single job, ticking all actors of a given UClass. Tick aggregation
	// of a given class ensure we load our virtual AActor::Tick call within register
	// and reduce cache misses created by random access of derived classes.
	struct FAVVMRunner_Actor
	{
		TArray<TWeakObjectPtr<AActor>> Entities;
	};
	
	struct FAVVMRunner_ActorComponent
	{
		TArray<TWeakObjectPtr<UActorComponent>> Entities;
	};

	struct FAVVMJobQueue
	{
		FAVVMJobQueue()
		{
			static constexpr int32 NumJobs_ActorComponent = 18;
			static constexpr int32 NumJobs_Actor = 12;
			Jobs_ActorComponent.Reserve(NumJobs_ActorComponent);
			Jobs_Actor.Reserve(NumJobs_Actor);
		}

		int64 Add(const UClass* Class, UActorComponent* ActorComponent);
		int64 Add(const UClass* Class, AActor* Actor);
		int64 Append(const UClass* Class, const TArray<TWeakObjectPtr<UActorComponent>>& ActorComponents);
		int64 Append(const UClass* Class, const TArray<TWeakObjectPtr<AActor>>& Actors);

		TMap<TWeakObjectPtr<const UClass>, FAVVMRunner_ActorComponent> Jobs_ActorComponent;
		/*Jobs are unique per AActor derived type. Priority is handled by indices*/
		TMap<TWeakObjectPtr<const UClass>, FAVVMRunner_Actor> Jobs_Actor;
	};

	struct FAVVMMLFQ
	{
		FAVVMMLFQ()
		{
			static constexpr int32 NumFeedbackLevels = 4;
			PriorityQueue.Reserve(NumFeedbackLevels);
		}

		int64 Push(const UClass* Class, UActorComponent* ActorComponent);
		int64 Push(const UClass* Class, AActor* Actor);
		void Pop(const int64 Handle, UActorComponent* ActorComponent) const;
		void Pop(const int64 Handle, AActor* Actor) const;

		/*Priority level is handled by indices*/
		TArray<FAVVMJobQueue> PriorityQueue;
	};
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UAVVMTickSchedulerRule> TickSchedulerRule = nullptr;

	TMap<TWeakObjectPtr<const UClass>, int64> TickerHandles;
	FAVVMMLFQ MultiLevelFeedbackQueue = FAVVMMLFQ();

	UPROPERTY(Transient, BlueprintReadOnly)
	float GlobalResetTimeJobQueuePriority = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	float GlobalJobAllotment = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	float TickRate = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	float ResetJobQueuePriorityDeltaTime = 0.f;
};
