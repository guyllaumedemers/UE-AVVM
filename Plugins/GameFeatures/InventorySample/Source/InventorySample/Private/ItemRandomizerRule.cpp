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
#include "ItemRandomizerRule.h"

#include "AVVMToolkitUtils.h"
#include "InventoryProvider.h"
#include "Engine/AssetManager.h"

const FGameplayTag& UItemRandomizerImpl::GetImplTag() const
{
	return CategoryTag;
}

#if WITH_EDITOR
EDataValidationResult UItemRandomizerRule::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ImplClasses.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UItemRandomizerRule", "", "Item Randomizer Impl is empty!"));
	}

	return Result;
}
#endif

void UItemRandomizerRule::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsTemplate())
	{
		return;
	}
	
	StreamableHandle.Reset();
	Impls.Reset();

	TArray<FSoftObjectPath> SoftObjectPaths;
	for (const auto& ImplClass : ImplClasses)
	{
		SoftObjectPaths.Add(ImplClass.ToSoftObjectPath());
	}

	const auto OnResourceAcquired = [](const TWeakObjectPtr<UItemRandomizerRule>& Rule)
	{
		if (!Rule.IsValid() || !Rule->StreamableHandle.IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		Rule->StreamableHandle->GetLoadedAssets(OutResources);

		for (UObject* Resource : OutResources)
		{
			auto* ImplClass = Cast<UClass>(Resource);
			if (!IsValid(ImplClass))
			{
				continue;
			}

			const auto* CDO = ImplClass->GetDefaultObject<UItemRandomizerImpl>();
			if (IsValid(CDO))
			{
				auto& OutResult = Rule->Impls.FindOrAdd(CDO->GetImplTag());
				OutResult = CDO;
			}
		}
	};

	const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnResourceAcquired, this);
	StreamableHandle = UAssetManager::Get().LoadAssetList(SoftObjectPaths, Callback);
}

TArray<UItemObject*> UItemRandomizerRule::GetRandomSubset(const AActor* Outer,
                                                          const TArray<UItemObject*>& Items) const
{
	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UInventoryProvider>(Outer);
	if (!bResult)
	{
		return {};
	}

	const FGameplayTag ImplCategory = IInventoryProvider::Execute_GetItemRandomizerRuleType(Outer);
	const auto* OutResult = Impls.Find(ImplCategory);
	if ((OutResult != nullptr) && OutResult->IsValid())
	{
		TArray<UItemObject*> OutResults = (*OutResult)->GetFilteredItems(Items);
		return OutResults;
	}

	return {};
}
