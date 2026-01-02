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
#include "AVVMGameplay.h"

#include "AVVMTickScheduler.h"
#include "DataRegistrySubsystem.h"
#include "GameplayTagsModule.h"
#include "Engine/AssetManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogGameplay);

static const FString TagPath = (FPaths::ProjectPluginsDir() / TEXT("AVVM/Config/Tags"));

TSharedPtr<IConsoleVariable> FAVVMGameplayModule::CVarEnableTickSchedulerSubsystem = nullptr;
TSharedPtr<IConsoleVariable> FAVVMGameplayModule::CVarEnableOverrideTickSchedulerRule = nullptr;
TSharedPtr<IConsoleVariable> FAVVMGameplayModule::CVarOverrideTickSchedulerJobAllotment = nullptr;
TSharedPtr<IConsoleVariable> FAVVMGameplayModule::CVarOverrideTickSchedulerTickRate = nullptr;

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

	IConsoleVariable* CVar_EnableTickSchedulerSubsystem = IConsoleManager::Get()
			.RegisterConsoleVariable(TEXT("TickScheduler.EnableShouldCreateSubsystem"),
			                         false,
			                         TEXT("Enable Tick Scheduler Subsystem creation."));

	IConsoleVariable* CVar_EnableOverrideTickSchedulerRule = IConsoleManager::Get()
			.RegisterConsoleVariable(TEXT("TickScheduler.EnableRuleOverride"),
			                         false,
			                         TEXT("Enable Tick Scheduler Rule Override."));

	IConsoleVariable* CVar_OverrideTickSchedulerJobAllotment = IConsoleManager::Get()
			.RegisterConsoleVariable(TEXT("TickScheduler.OverrideJobAllotment"),
			                         0.f,
			                         TEXT("Enable Job Allotment Override."));

	IConsoleVariable* CVar_OverrideTickSchedulerTickRate = IConsoleManager::Get()
			.RegisterConsoleVariable(TEXT("TickScheduler.OverrideTickRate"),
			                         0.f,
			                         TEXT("Enable TickRate Override."));

	CVarEnableTickSchedulerSubsystem = MakeShareable<IConsoleVariable>(CVar_EnableTickSchedulerSubsystem);
	CVarEnableOverrideTickSchedulerRule = MakeShareable<IConsoleVariable>(CVar_EnableOverrideTickSchedulerRule);
	CVarOverrideTickSchedulerJobAllotment = MakeShareable<IConsoleVariable>(CVar_OverrideTickSchedulerJobAllotment);
	CVarOverrideTickSchedulerTickRate = MakeShareable<IConsoleVariable>(CVar_OverrideTickSchedulerTickRate);

#if !UE_BUILD_SHIPPING
	if (CVarEnableOverrideTickSchedulerRule.IsValid())
	{
		auto& Delegate = CVarEnableOverrideTickSchedulerRule->OnChangedDelegate();
		HandleA = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}

	if (CVarOverrideTickSchedulerJobAllotment.IsValid())
	{
		auto& Delegate = CVarOverrideTickSchedulerJobAllotment->OnChangedDelegate();
		HandleB = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}

	if (CVarOverrideTickSchedulerTickRate.IsValid())
	{
		auto& Delegate = CVarOverrideTickSchedulerTickRate->OnChangedDelegate();
		HandleC = Delegate.AddStatic(NSAVVMFunctor::OnTickSchedulerCVarChanged);
	}
#endif
}

void FAVVMGameplayModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(CVarEnableTickSchedulerSubsystem.Get());
	IConsoleManager::Get().UnregisterConsoleObject(CVarEnableOverrideTickSchedulerRule.Get());
	IConsoleManager::Get().UnregisterConsoleObject(CVarOverrideTickSchedulerJobAllotment.Get());
	IConsoleManager::Get().UnregisterConsoleObject(CVarOverrideTickSchedulerTickRate.Get());

#if !UE_BUILD_SHIPPING
	if (CVarEnableOverrideTickSchedulerRule.IsValid())
	{
		auto& Delegate = CVarEnableOverrideTickSchedulerRule->OnChangedDelegate();
		Delegate.Remove(HandleA);
	}

	if (CVarOverrideTickSchedulerJobAllotment.IsValid())
	{
		auto& Delegate = CVarOverrideTickSchedulerJobAllotment->OnChangedDelegate();
		Delegate.Remove(HandleB);
	}

	if (CVarOverrideTickSchedulerTickRate.IsValid())
	{
		auto& Delegate = CVarOverrideTickSchedulerTickRate->OnChangedDelegate();
		Delegate.Remove(HandleC);
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

TSharedRef<IConsoleVariable> FAVVMGameplayModule::GetCVarEnableTickSchedulerSubsystem()
{
	return CVarEnableTickSchedulerSubsystem.ToSharedRef();
}

TSharedRef<IConsoleVariable> FAVVMGameplayModule::GetCVarEnableOverrideTickSchedulerRule()
{
	return CVarEnableOverrideTickSchedulerRule.ToSharedRef();
}

TSharedRef<IConsoleVariable> FAVVMGameplayModule::GetCVarOverrideTickSchedulerJobAllotment()
{
	return CVarOverrideTickSchedulerJobAllotment.ToSharedRef();
}

TSharedRef<IConsoleVariable> FAVVMGameplayModule::GetCVarOverrideTickSchedulerTickRate()
{
	return CVarOverrideTickSchedulerTickRate.ToSharedRef();
}

IMPLEMENT_MODULE(FAVVMGameplayModule, AVVMGameplay)
