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

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "Ability/AVVMAbilityData.h"
#include "Engine/AssetManager.h"

void UAVVMAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningOuter = GetTypedOuter<AActor>();
}

void UAVVMAbilitySystemComponent::SetupAbilities(const TArray<UObject*>& Resources)
{
	const FGameplayTagContainer& ASCTags = GetOwnedGameplayTags();

	const AActor* Outer = OwningOuter.Get();
	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintIsServerOrClient(Outer).GetData();

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
			UE_LOG(LogGameplay,
			       Log,
			       TEXT("Executed from \"%s\". Failed to Grant Ability \"%s\" due to Blocking Tags."),
			       IsServerOrClientString,
			       *AbilityAsset->GetName());

			continue;
		}

		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". New Ability Recorded \"%s\" for deferred Granting."),
		       IsServerOrClientString,
		       *AbilityAsset->GetName());

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
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Actor!")))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintIsServerOrClient(Outer).GetData();

	TArray<UObject*> OutStreamedAssets;
	(*OutResult)->GetLoadedAssets(OutStreamedAssets);

	for (UObject* Ability : OutStreamedAssets)
	{
		auto* GameplayAbilityClass = Cast<UClass>(Ability);
		if (!IsValid(GameplayAbilityClass))
		{
			continue;
		}

		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Granting New Ability \"%s\" to Actor \"%s\"."),
		       IsServerOrClientString,
		       *GameplayAbilityClass->GetName(),
		       *Outer->GetName());

		GiveAbility(FGameplayAbilitySpec{GameplayAbilityClass});
	}
}

void UAVVMAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
	Super::OnTagUpdated(Tag, TagExists);

	const AActor* Outer = OwningOuter.Get();
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Modifying Tag \"%s\"."),
	       UAVVMGameplayUtils::PrintIsServerOrClient(Outer).GetData(),
	       *Tag.ToString());
}
