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
#include "AVVMWorldSetting.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

const FGameplayTag& UAVVMWorldRule::GetRuleTag() const
{
	return RuleTag;
}

void AAVVMWorldSetting::BeginPlay()
{
	Super::BeginPlay();

	const TArray<FSoftObjectPath> RulePaths = GetRulePaths();
	AsyncLoadWorldRules(RulePaths);
}

void AAVVMWorldSetting::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	StreamableHandle.Reset();
	Rules.Reset();
}

UAVVMWorldRule* AAVVMWorldSetting::GetRule(const FGameplayTag& RuleTag) const
{
	const bool bDoesContains = Rules.Contains(RuleTag);
	if (bDoesContains)
	{
		return Rules[RuleTag];
	}
	else
	{
		return nullptr;
	}
}

TArray<FSoftObjectPath> AAVVMWorldSetting::GetRulePaths() const
{
	TArray<FSoftObjectPath> SoftObjectPaths;
	for (const auto [Tag, RuleClass] : RuleClassPerTag)
	{
		if (RuleClass.IsNull())
		{
			continue;
		}
		else
		{
			SoftObjectPaths.Add(RuleClass.ToSoftObjectPath());
		}
	}

	return SoftObjectPaths;
}

void AAVVMWorldSetting::AsyncLoadWorldRules(const TArray<FSoftObjectPath>& SoftObjectPaths)
{
	const auto CreateRules = [](const TWeakObjectPtr<AAVVMWorldSetting>& Caller)
	{
		if (!Caller.IsValid())
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = Caller->StreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		Handle->GetLoadedAssets(OutResources);

		for (UObject* Resource : OutResources)
		{
			auto* RuleClass = Cast<UClass>(Resource);
			if (!IsValid(RuleClass))
			{
				continue;
			}

			auto* Rule = NewObject<UAVVMWorldRule>(Caller.Get(), RuleClass);
			if (!IsValid(Rule))
			{
				continue;
			}

			TObjectPtr<UAVVMWorldRule>& Out = Caller->Rules.FindOrAdd(Rule->GetRuleTag());
			Out = Rule;
		}
	};

	const auto Callback = FStreamableDelegate::CreateWeakLambda(this, CreateRules, TWeakObjectPtr(this));
	StreamableHandle = UAssetManager::Get().LoadAssetList(SoftObjectPaths, Callback);
}
