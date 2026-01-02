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
#include "AVVMTickScheduler.h"

#include "AVVMDoesSupportManualTicking.h"
#include "AVVMGameplay.h"
#include "AVVMTickSchedulerRule.h"
#include "AVVMUtils.h"
#include "AVVMWorldSetting.h"
#include "NativeGameplayTags.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_WORLD_RULE_TICK_SCHEDULING, "WorldRule.TickScheduling");

bool UAVVMTickScheduler::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (!IsValid(World))
	{
		return false;
	}

	const auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (!IsValid(WorldSettings) || !WorldSettings->DoesProjectRuleClassExist(UAVVMTickSchedulerRule::StaticClass()))
	{
		return false;
	}

	const bool bShouldCreateSubsystem = FAVVMGameplayModule::GetCVarEnableTickSchedulerSubsystem()->GetBool();
	return bShouldCreateSubsystem;
}

void UAVVMTickScheduler::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// @gdemers Project Rules are defined in your game project settings, and are loaded by the WorldSetting actors OnBeginPlay.
	GetSetProjectTickSchedulerRule();
}

void UAVVMTickScheduler::Deinitialize()
{
	Super::Deinitialize();
}

void UAVVMTickScheduler::Tick(float DeltaTime)
{
	// @gdemers entries subject to priority changes.
	TArray<FAVVMJobQueue> LongRunningJobs;
	LongRunningJobs.Reserve(MultiLevelFeedbackQueue.PriorityQueue.Num());

	// @gdemers we ensure that any addition/removal during the frame won't conflict with existing entry during our tick.
	TArray<FAVVMJobQueue> TempJobQueue = MoveTemp(MultiLevelFeedbackQueue.PriorityQueue);
	int32 NextPriorityLevel = 0;

	// @gdemers timestamp tick begin.
	const double TickBegin = FPlatformTime::Seconds();

	// @gdemers go over all priority level, starting at highest (i.e - 0). 
	for (auto JobQueueIterator = TempJobQueue.CreateIterator(); JobQueueIterator; ++JobQueueIterator)
	{
		// @gdemers clamp index.
		NextPriorityLevel = FMath::Clamp(NextPriorityLevel + 1, 0, TempJobQueue.Num() - 1);

		// @gdemers execute tick on all actors of class.
		for (auto& [Class, Runner_Actor] : JobQueueIterator->Jobs_Actor)
		{
			if (Runner_Actor.Entities.IsEmpty())
			{
				continue;
			}

			// @gdemers timestamp before executing job.
			const double Before = FPlatformTime::Seconds();

			// @gdemers run job for the given actor type. our expectation is that the tick process
			// is small enough to keep this actor type at the highest level of priority.
			for (auto Iterator = Runner_Actor.Entities.CreateIterator(); Iterator; ++Iterator)
			{
				AActor* Actor = Iterator->Get();
				if (!IsValid(Actor))
				{
					Iterator.RemoveCurrentSwap();
					continue;
				}

				Actor->Tick(DeltaTime);

				// @gdemers timestamp after executing job.
				const double Now = FPlatformTime::Seconds();

				// @gdemers was the job longer than the allotment defined by user, and should we
				// lower our priority due to long-running job.
				const bool bWasLongRunningProcess = ((Now - Before) > GlobalJobAllotment);
				if (bWasLongRunningProcess)
				{
					// @gdemers we already ticked our actor, the next index was skipped. we need to swap entries to ensure all actors will eventually update.
					// if not now, on next frame,
					const int32 NextIndex = ((Iterator.GetIndex() + 1) % Runner_Actor.Entities.Num());
					Algo::Rotate(Runner_Actor.Entities, NextIndex);

					// @gdemers ensure execution ordering, preventing Move semantic to be discarded by compiler when doing argument inlining.
					TArray<TWeakObjectPtr<AActor>> TempActors = MoveTemp(Runner_Actor.Entities);
					LongRunningJobs[NextPriorityLevel].Append(Class.Get(), TempActors);
					break;
				}
			}
		}

		// @gdemers component may require to react to actor position updates, and are bound to actor so
		// they should be updated after the actor.
		// Note : Ticking dependent components shouldnt be required right after parent updates. Otherwise, we suffer hops that counter-effect
		// the optimization in place.
		for (auto& [Class, Runner_ActorComponent] : JobQueueIterator->Jobs_ActorComponent)
		{
			if (Runner_ActorComponent.Entities.IsEmpty())
			{
				continue;
			}

			const double Before = FPlatformTime::Seconds();

			for (auto Iterator = Runner_ActorComponent.Entities.CreateIterator(); Iterator; ++Iterator)
			{
				UActorComponent* Component = Iterator->Get();
				if (!IsValid(Component))
				{
					Iterator.RemoveCurrentSwap();
					continue;
				}

				Component->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);

				const double Now = FPlatformTime::Seconds();

				const bool bWasLongRunningProcess = ((Now - Before) > GlobalJobAllotment);
				if (bWasLongRunningProcess)
				{
					const int32 NextIndex = ((Iterator.GetIndex() + 1) % Runner_ActorComponent.Entities.Num());
					Algo::Rotate(Runner_ActorComponent.Entities, NextIndex);

					TArray<TWeakObjectPtr<UActorComponent>> TempActorComponents = MoveTemp(Runner_ActorComponent.Entities);
					LongRunningJobs[NextPriorityLevel].Append(Class.Get(), TempActorComponents);
					break;
				}
			}
		}

		// @gdemers we may want to split processing our tick data over multiple frame to ensure more consistent frame rate.
		const double TickNow = FPlatformTime::Seconds();
		const bool bWasLongTick = ((TickNow - TickBegin) > TickRate);
		if (bWasLongTick)
		{
			break;
		}
	}

	// @gdemers start from back since we last executed those entries, and are likely to be already in register caches.
	for (int32 i = TempJobQueue.Num() - 1; i >= 0; --i)
	{
		// @gdemers push back everything in the priority queue with new placement.
		// IMPORTANT : Do not reset, content may have changed between the start, and end of this function for our Actors,
		// and their components.
		FAVVMJobQueue& Dest = MultiLevelFeedbackQueue.PriorityQueue[i];

		for (auto& [Class, Runner_Actor] : TempJobQueue[i].Jobs_Actor)
		{
			const bool bDoesContains = LongRunningJobs[i].Jobs_Actor.Contains(Class);
			if (bDoesContains)
			{
				TArray<TWeakObjectPtr<AActor>> TempActors = MoveTemp(LongRunningJobs[i].Jobs_Actor[Class].Entities);
				const int32 NewHandle = Dest.Append(Class.Get(), TempActors);
				TickerHandles.FindOrAdd(Class, NewHandle);
			}
		}

		for (auto& [Class, Runner_ActorComponent] : TempJobQueue[i].Jobs_ActorComponent)
		{
			const bool bDoesContains = LongRunningJobs[i].Jobs_ActorComponent.Contains(Class);
			if (bDoesContains)
			{
				TArray<TWeakObjectPtr<UActorComponent>> TempActorComponents = MoveTemp(LongRunningJobs[i].Jobs_ActorComponent[Class].Entities);
				const int32 NewHandle = Dest.Append(Class.Get(), TempActorComponents);
				TickerHandles.FindOrAdd(Class, NewHandle);
			}
		}
	}

	// @gdemers after a period of time, we want all job queues to be put at the highest priority,
	// and grant longer process a chance to run on the cpu again.
	ResetJobQueuePriorityDeltaTime += DeltaTime;
	if (ResetJobQueuePriorityDeltaTime > GlobalResetTimeJobQueuePriority)
	{
		ResetJobQueuePriorityDeltaTime = 0.f;

		const int32 Count = MultiLevelFeedbackQueue.PriorityQueue.Num();
		for (int32 i = 1; i < Count; ++i)
		{
			FAVVMJobQueue& Dest = MultiLevelFeedbackQueue.PriorityQueue[0];
			FAVVMJobQueue& Src = MultiLevelFeedbackQueue.PriorityQueue[i];

			for (auto& [Class, Runner_Actor] : Src.Jobs_Actor)
			{
				TArray<TWeakObjectPtr<AActor>> TempActors = MoveTemp(Src.Jobs_Actor[Class].Entities);
				const int32 NewHandle = Dest.Append(Class.Get(), TempActors);
				TickerHandles.FindOrAdd(Class, NewHandle);
			}

			for (auto& [Class, Runner_ActorComponent] : Src.Jobs_ActorComponent)
			{
				TArray<TWeakObjectPtr<UActorComponent>> TempActorComponents = MoveTemp(Src.Jobs_ActorComponent[Class].Entities);
				const int32 NewHandle = Dest.Append(Class.Get(), TempActorComponents);
				TickerHandles.FindOrAdd(Class, NewHandle);
			}
		}
	}
}

void UAVVMTickScheduler::Static_Register(const UWorld* World,
                                         const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTickActor)
{
	auto* Scheduler = UAVVMTickScheduler::Get(World);
	if (IsValid(Scheduler))
	{
		Scheduler->Register(ManualTickActor);
	}
}

void UAVVMTickScheduler::Static_UnRegister(const UWorld* World,
                                           const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTickActor)
{
	auto* Scheduler = UAVVMTickScheduler::Get(World);
	if (IsValid(Scheduler))
	{
		Scheduler->UnRegister(ManualTickActor);
	}
}

#if !UE_BUILD_SHIPPING
void UAVVMTickScheduler::OnTickSchedulerRuleCVarChanged()
{
	auto* World = GetTypedOuter<UWorld>();
	if (IsValid(World))
	{
		const auto Callback = FTimerDelegate::CreateUObject(this, &UAVVMTickScheduler::InitRule);
		World->GetTimerManager().SetTimerForNextTick(Callback);
	}
}
#endif

UAVVMTickScheduler* UAVVMTickScheduler::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UAVVMTickScheduler>(World);
}

void UAVVMTickScheduler::Register(const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTicker)
{
	if (!UAVVMUtils::IsNativeScriptInterfaceValid(ManualTicker))
	{
		return;
	}

	// @gdemers Notes : New jobs entering the system should be placed at the higher priority, and
	// moved based on allotment time spent during execution time.
	auto* Actor = Cast<AActor>(ManualTicker.GetObject());
	if (IsValid(Actor))
	{
		const UClass* Class = Actor->GetClass();
		const int64 OutHandle = MultiLevelFeedbackQueue.Push(Class, Actor);
		TickerHandles.FindOrAdd(Class, OutHandle);
		return;
	}

	auto* ActorComponent = Cast<UActorComponent>(ManualTicker.GetObject());
	if (IsValid(ActorComponent))
	{
		const UClass* Class = ActorComponent->GetClass();
		const int64 OutHandle = MultiLevelFeedbackQueue.Push(Class, ActorComponent);
		TickerHandles.FindOrAdd(Class, OutHandle);
		return;
	}

	ensureAlwaysMsgf(IsValid(Actor) || IsValid(ActorComponent),
	                 TEXT("Tick aggregation is only supported on AActor derived types, and UActorComponent derived types."));
}

void UAVVMTickScheduler::UnRegister(const TScriptInterface<IAVVMDoesSupportManualTicking>& ManualTicker)
{
	if (!UAVVMUtils::IsNativeScriptInterfaceValid(ManualTicker))
	{
		return;
	}

	auto* Actor = Cast<AActor>(ManualTicker.GetObject());
	if (IsValid(Actor))
	{
		const UClass* Class = Actor->GetClass();
		const bool bDoesContains = TickerHandles.Contains(Class);
		if (bDoesContains)
		{
			const int64 OutHandle = TickerHandles[Class];
			MultiLevelFeedbackQueue.Pop(OutHandle, Actor);
		}

		return;
	}

	auto* ActorComponent = Cast<UActorComponent>(ManualTicker.GetObject());
	if (IsValid(ActorComponent))
	{
		const UClass* Class = Actor->GetClass();
		const bool bDoesContains = TickerHandles.Contains(Class);
		if (bDoesContains)
		{
			const int64 OutHandle = TickerHandles[Class];
			MultiLevelFeedbackQueue.Pop(OutHandle, ActorComponent);
		}

		return;
	}

	ensureAlwaysMsgf(IsValid(Actor) || IsValid(ActorComponent),
	                 TEXT("Tick aggregation is only supported on AActor derived types, and UActorComponent derived types."));
}

void UAVVMTickScheduler::GetSetProjectTickSchedulerRule()
{
	const auto* World = GetTypedOuter<UWorld>();
	if (!IsValid(World))
	{
		return;
	}

	auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (IsValid(WorldSettings))
	{
		TickSchedulerRule = WorldSettings->GetRule<UAVVMTickSchedulerRule>(TAG_WORLD_RULE_TICK_SCHEDULING);
		InitRule();
	}
}

void UAVVMTickScheduler::InitRule()
{
	const UAVVMTickSchedulerRule* Rule = TickSchedulerRule.Get();
	if (IsValid(Rule))
	{
		GlobalResetTimeJobQueuePriority = Rule->GetGlobalResetTimeJobQueuePriority();
		GlobalJobAllotment = Rule->GetGlobalJobAllotment();
		TickRate = Rule->GetTickRate();
	}
}

int64 UAVVMTickScheduler::FAVVMJobQueue::Add(const UClass* Class, UActorComponent* ActorComponent)
{
	FAVVMRunner_ActorComponent& OutResult = Jobs_ActorComponent.FindOrAdd(Class);
	OutResult.Entities.AddUnique(ActorComponent);
	return reinterpret_cast<int64>(&OutResult.Entities);
}

int64 UAVVMTickScheduler::FAVVMJobQueue::Add(const UClass* Class, AActor* Actor)
{
	FAVVMRunner_Actor& OutResult = Jobs_Actor.FindOrAdd(Class);
	OutResult.Entities.AddUnique(Actor);
	return reinterpret_cast<int64>(&OutResult.Entities);
}

int64 UAVVMTickScheduler::FAVVMJobQueue::Append(const UClass* Class, const TArray<TWeakObjectPtr<UActorComponent>>& ActorComponents)
{
	FAVVMRunner_ActorComponent& Runner_ActorComponent = Jobs_ActorComponent.FindOrAdd(Class);
	Runner_ActorComponent.Entities.Append(ActorComponents);
	return reinterpret_cast<int64>(&Jobs_ActorComponent[Class].Entities);
}

int64 UAVVMTickScheduler::FAVVMJobQueue::Append(const UClass* Class, const TArray<TWeakObjectPtr<AActor>>& Actors)
{
	FAVVMRunner_Actor& Runner_Actor = Jobs_Actor.FindOrAdd(Class);
	Runner_Actor.Entities.Append(Actors);
	return reinterpret_cast<int64>(&Jobs_Actor[Class].Entities);
}

int64 UAVVMTickScheduler::FAVVMMLFQ::Push(const UClass* Class, UActorComponent* ActorComponent)
{
	if (!IsValid(Class) || !IsValid(ActorComponent))
	{
		return INDEX_NONE;
	}

	FAVVMJobQueue& JobQueue = PriorityQueue[0];
	const int64 OutHandle = JobQueue.Add(Class, ActorComponent);
	return OutHandle;
}

int64 UAVVMTickScheduler::FAVVMMLFQ::Push(const UClass* Class, AActor* Actor)
{
	if (!IsValid(Class) || !IsValid(Actor))
	{
		return INDEX_NONE;
	}

	// @gdemers we ALWAYS want new jobs to push onto the highest level.
	FAVVMJobQueue& JobQueue = PriorityQueue[0];
	const int64 OutHandle = JobQueue.Add(Class, Actor);
	return OutHandle;
}

void UAVVMTickScheduler::FAVVMMLFQ::Pop(const int64 Handle, UActorComponent* ActorComponent) const
{
	auto* PtrToArray = reinterpret_cast<TArray<TWeakObjectPtr<UActorComponent>>*>(Handle);
	if (ensureAlwaysMsgf(PtrToArray != nullptr,
	                     TEXT("Invalid Handle. Verify that the handle is pointing to the collection type we expect to use during removal process.")))
	{
		PtrToArray->Remove(ActorComponent);
	}
}

void UAVVMTickScheduler::FAVVMMLFQ::Pop(const int64 Handle, AActor* Actor) const
{
	auto* PtrToArray = reinterpret_cast<TArray<TWeakObjectPtr<AActor>>*>(Handle);
	if (ensureAlwaysMsgf(PtrToArray != nullptr,
	                     TEXT("Invalid Handle. Verify that the handle is pointing to the collection type we expect to use during removal process.")))
	{
		PtrToArray->Remove(Actor);
	}
}
