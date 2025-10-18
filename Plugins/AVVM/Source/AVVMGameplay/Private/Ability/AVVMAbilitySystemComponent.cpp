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
#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Ability/AVVMGameplayAbility.h"
#include "Engine/AssetManager.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UAVVMAbilitySystemComponent_InstanceCounter, TEXT("Ability System Component Instance Counter"));

void UAVVMAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMAbilitySystemComponent_InstanceCounter);

	OwningOuter = Outer;

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAVVMAbilitySystemComponent::StaticClass()->GetName(),
	       *Outer->GetName());
}

void UAVVMAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles)
	{
		ClearAbility(AbilitySpecHandle);
	}
	
	AbilityHandleSystem.Reset();
	AttributeSetHandle.Reset();
	AbilitySpecHandles.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAVVMAbilitySystemComponent::StaticClass()->GetName(),
	       *Outer->GetName());

	OwningOuter.Reset();
}

void UAVVMAbilitySystemComponent::SetupAbilities(const TArray<UObject*>& Resources)
{
	const FGameplayTagContainer& OwnedTags = GetOwnedGameplayTags();

	const AActor* Outer = OwningOuter.Get();
	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<FSoftObjectPath> DeferredGrantedAbilities;
	for (const UObject* Resource : Resources)
	{
		const auto* AbilityAsset = Cast<UAVVMAbilityDefinitionDataAsset>(Resource);
		if (!IsValid(AbilityAsset))
		{
			continue;
		}

		if (!AbilityAsset->CanGrantAbility(OwnedTags, OwnedTags))
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
		Callback.BindUObject(this, &UAVVMAbilitySystemComponent::OnAbilityGrantingDeferred, Token);

		TSharedPtr<FStreamableHandle>& OutResult = AbilityHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredGrantedAbilities, Callback);
	}
}

void UAVVMAbilitySystemComponent::SetupAttributeSet(const FSoftObjectPath& AttributeSetSoftObjectPath)
{
	const auto OnAsyncRequestComplete = [](const TWeakObjectPtr<UAVVMAbilitySystemComponent>& Caller)
	{
		UAVVMAbilitySystemComponent* ASC = Caller.Get();
		if (!IsValid(ASC))
		{
			return;
		}

		TSharedPtr<FStreamableHandle> StreamableHandle = ASC->AttributeSetHandle;
		if (!StreamableHandle.IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		StreamableHandle->GetLoadedAssets(OutResources);

		for (UObject* Resource : OutResources)
		{
			auto* AttributeSetClass = Cast<UClass>(Resource);
			if (IsValid(AttributeSetClass))
			{
				ASC->GetOrCreateAttributeSubobject(AttributeSetClass);
			}
		}
	};

	FStreamableDelegate Callback;
	Callback.BindWeakLambda(this, OnAsyncRequestComplete, TWeakObjectPtr(this));
	AttributeSetHandle = UAssetManager::Get().LoadAssetList({AttributeSetSoftObjectPath}, Callback);
}

void UAVVMAbilitySystemComponent::OnAbilityGrantingDeferred(FAbilityToken AbilityToken)
{
	const TSharedPtr<FStreamableHandle>* OutResult = AbilityHandleSystem.Find(AbilityToken.UniqueId);
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<UObject*> OutStreamedAssets;
	(*OutResult)->GetLoadedAssets(OutStreamedAssets);

	for (UObject* StreamableAsset : OutStreamedAssets)
	{
		auto* GameplayAbilityClass = Cast<UClass>(StreamableAsset);
		if (!IsValid(GameplayAbilityClass))
		{
			continue;
		}

		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Granting New Ability \"%s\" on Outer \"%s\"."),
		       IsServerOrClientString,
		       *GameplayAbilityClass->GetName(),
		       *Outer->GetName());

		const FGameplayAbilitySpecHandle NewAbilitySpecHandle = GiveAbility(FGameplayAbilitySpec{
			GameplayAbilityClass,
			1,
			GameplayAbilityClass->GetDefaultObject<UAVVMGameplayAbility>()->GetInputId()
		});

		AbilitySpecHandles.Add(NewAbilitySpecHandle);
	}
}

void UAVVMAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
	Super::OnTagUpdated(Tag, TagExists);

	const AActor* Outer = OwningOuter.Get();
	if (!IsValid(Outer))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Modifying Tag \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *Tag.ToString(),
	       *Outer->GetName());
}
