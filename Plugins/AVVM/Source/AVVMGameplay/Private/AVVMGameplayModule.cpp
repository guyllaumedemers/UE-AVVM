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
#include "AVVMGameplayModule.h"

#include "AVVMTickScheduler.h"
#include "DataRegistrySubsystem.h"
#include "GameplayTagsModule.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogGameplay);

static const FString TagPath = (FPaths::ProjectPluginsDir() / TEXT("AVVM/Config/Tags"));

// @gdemers global console commands to be configured through user console cmd, or .ini file.
static int32 CVarEnableOverrideTickSchedulerRule = 0;
static FAutoConsoleVariableRef CEnableOverrideTickSchedulerRule(TEXT("c.SetTickSchedulerRule"),
                                                                CVarEnableOverrideTickSchedulerRule,
                                                                TEXT("0, or 1 for overwriting the WorldRule running"),
                                                                ECVF_Default);

static float CVarOverrideTickSchedulerJobAllotment = 0;
static FAutoConsoleVariableRef COverrideTickSchedulerJobAllotment(TEXT("c.SetTickSchedulerJobAllotment"),
                                                                  CVarOverrideTickSchedulerJobAllotment,
                                                                  TEXT("Set job allotment time"),
                                                                  ECVF_Default);

static float CVarOverrideTickSchedulerTickRate = 0;
static FAutoConsoleVariableRef COverrideTickSchedulerTickRate(TEXT("c.SetTickSchedulerTickRate"),
                                                              CVarOverrideTickSchedulerTickRate,
                                                              TEXT("Set tick rate"),
                                                              ECVF_Default);

static float CVarOverrideTickSchedulerResetTimeJobQueuePriority = 0;
static FAutoConsoleVariableRef COverrideTickSchedulerResetTimeJobQueuePriority(TEXT("c.SetTickSchedulerResetTimeJobQueuePriority"),
                                                                               CVarOverrideTickSchedulerResetTimeJobQueuePriority,
                                                                               TEXT("Set time for priority queue reset. i.e put all jobs back to highest level."),
                                                                               ECVF_Default);

namespace NSAVVMFunctor
{
#if !UE_BUILD_SHIPPING
	void OnTickSchedulerCVarChanged(IConsoleVariable* NewCVar)
	{
		if (!IsValid(GEngine))
		{
			return;
		}

		auto* TickScheduler = UWorld::GetSubsystem<UAVVMTickScheduler>(GEngine->GetWorld());
		if (IsValid(TickScheduler))
		{
			TickScheduler->OnTickSchedulerRuleCVarChanged();
		}
	}
#endif
}

void FAVVMGameplayModule::StartupModule()
{
	const IGameplayTagsModule& GameplayTagModule = IGameplayTagsModule::Get();
	if (GameplayTagModule.IsAvailable())
	{
		UGameplayTagsManager::Get().AddTagIniSearchPath(TagPath);
	}

#if !UE_BUILD_SHIPPING
	{
		auto& Delegate = CEnableOverrideTickSchedulerRule->OnChangedDelegate();
		HandleA = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}

	{
		auto& Delegate = COverrideTickSchedulerJobAllotment->OnChangedDelegate();
		HandleB = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}

	{
		auto& Delegate = COverrideTickSchedulerTickRate->OnChangedDelegate();
		HandleC = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}

	{
		auto& Delegate = COverrideTickSchedulerResetTimeJobQueuePriority->OnChangedDelegate();
		HandleD = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}
#endif
}

void FAVVMGameplayModule::ShutdownModule()
{
#if !UE_BUILD_SHIPPING
	{
		auto& Delegate = CEnableOverrideTickSchedulerRule->OnChangedDelegate();
		Delegate.Remove(HandleA);
	}

	{
		auto& Delegate = COverrideTickSchedulerJobAllotment->OnChangedDelegate();
		Delegate.Remove(HandleB);
	}

	{
		auto& Delegate = COverrideTickSchedulerTickRate->OnChangedDelegate();
		Delegate.Remove(HandleC);
	}

	{
		auto& Delegate = COverrideTickSchedulerResetTimeJobQueuePriority->OnChangedDelegate();
		Delegate.Remove(HandleD);
	}
#endif

	const bool bIsAvailable = IGameplayTagsModule::IsAvailable();
	if (!bIsAvailable)
	{
		return;
	}

	UGameplayTagsManager* GameplayTagManager = UGameplayTagsManager::GetIfAllocated();
	if (IsValid(GameplayTagManager))
	{
		GameplayTagManager->RemoveTagIniSearchPath(TagPath);
	}
}

UDataRegistry* FAVVMGameplayModule::GetSetAutomatedTestDataRegistry()
{
	static TStrongObjectPtr<UDataRegistry> AutomatedTestDataRegistry = nullptr;
	if (!AutomatedTestDataRegistry.IsValid())
	{
		// TODO @gdemers require fallback. we should always have our test case available,
		// else we should create them.
	}

	return AutomatedTestDataRegistry.Get();
}

IConsoleVariable* FAVVMGameplayModule::GetCVarEnableOverrideTickSchedulerRule()
{
	return CEnableOverrideTickSchedulerRule->AsVariable();
}

IConsoleVariable* FAVVMGameplayModule::GetCVarOverrideTickSchedulerJobAllotment()
{
	return COverrideTickSchedulerJobAllotment->AsVariable();
}

IConsoleVariable* FAVVMGameplayModule::GetCVarOverrideTickSchedulerTickRate()
{
	return COverrideTickSchedulerTickRate->AsVariable();
}

IConsoleVariable* FAVVMGameplayModule::GetCVarOverrideTickSchedulerResetTimeJobQueuePriority()
{
	return COverrideTickSchedulerResetTimeJobQueuePriority->AsVariable();
}

IMPLEMENT_MODULE(FAVVMGameplayModule, AVVMGameplay)
