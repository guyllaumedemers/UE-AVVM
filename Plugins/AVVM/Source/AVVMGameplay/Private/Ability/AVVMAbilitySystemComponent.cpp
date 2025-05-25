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
#include "Ability/AVVMAbilitySystemComponent.h"

#include "AVVM.h"
#include "Ability/AVVMAbilityData.h"
#include "Engine/AssetManager.h"

void UAVVMAbilitySystemComponent::SetupAbilities(const TArray<UObject*>& Resources)
{
	const FGameplayTagContainer& ASCTags = GetOwnedGameplayTags();

	TArray<FSoftObjectPath> DeferredGrantedAbilities;
	for (const UObject* Resource : Resources)
	{
		const auto* AbilityAsset = Cast<UAVVMAbilityDataAsset>(Resource);
		if (!IsValid(AbilityAsset))
		{
			continue;
		}

		if (!AbilityAsset->CanGrantAbility(ASCTags))
		{
			UE_LOG(LogUI, Log, TEXT("Failed to Grant Ability due to Blocking Tags: %s"), *AbilityAsset->GetName());
			continue;
		}

		UE_LOG(LogUI, Log, TEXT("New Ability Recorded for deferred Granting: %s"), *AbilityAsset->GetName());
		DeferredGrantedAbilities.Add(AbilityAsset->GetGameplayAbilityClass().ToSoftObjectPath());
	}

	if (!DeferredGrantedAbilities.IsEmpty())
	{
		FAbilityToken Token;
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMAbilitySystemComponent::OnAbilityGrantedDeferred, Token);

		TSharedPtr<FStreamableHandle>& OutResult = AbilityHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredGrantedAbilities, Callback);
	}
}

void UAVVMAbilitySystemComponent::OnAbilityGrantedDeferred(FAbilityToken AbilityToken)
{
	const TSharedPtr<FStreamableHandle>* OutResult = AbilityHandleSystem.Find(AbilityToken.UniqueId);
	if (ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		TArray<UObject*> OutStreamedAssets;
		(*OutResult)->GetLoadedAssets(OutStreamedAssets);
		GrantAbilities(OutStreamedAssets);
	}
}

void UAVVMAbilitySystemComponent::GrantAbilities(const TArray<UObject*>& Abilities)
{
}

void UAVVMAbilitySystemComponent::GrantAbility(UGameplayAbility* Ability)
{
}
