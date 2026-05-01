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

#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "AVVMToolkitUtils.h"
#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Ability/AVVMAttributeSet.h"
#include "Ability/AVVMGameplayAbility.h"
#include "Engine/AssetManager.h"
#include "ProfilingDebugging/CountersTrace.h"

#if !UE_BUILD_SHIPPING
#include "AutomatedTest/AVVMAutomatedTestResourceValidationManager.h"
#include "Misc/AutomationTest.h"
#endif

TRACE_DECLARE_INT_COUNTER(UAVVMAbilitySystemComponent_InstanceCounter, TEXT("Ability System Component Instance Counter"));

UAVVMAbilitySystemComponent::UAVVMAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bAllowTickBatching = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	SetIsReplicatedByDefault(true);
}

void UAVVMAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_AUTOMATION_TESTS
	UAVVMAutomatedTestResourceValidationManager::Static_RegisterComponent(GetWorld(), this);
#endif

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMAbilitySystemComponent_InstanceCounter);
	AVVM_LOGGER_LOG(LogGameplay,
					Outer,
					Outer,
					TEXT("Adding %s."),
					*GetNameSafe(UAVVMAbilitySystemComponent::StaticClass()));

	OwningOuter = Outer;
}

void UAVVMAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_AUTOMATION_TESTS
	UAVVMAutomatedTestResourceValidationManager::Static_UnregisterComponent(GetWorld(), this);
#endif

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles)
	{
		ClearAbility(AbilitySpecHandle);
	}

	// @gdemers enforce cancelling running async process during actor destruction.
	for (auto Iterator = AbilityHandleSystem.CreateIterator(); Iterator; ++Iterator)
	{
		const TSharedPtr<FStreamableHandle>& Tuple = Iterator->Value;
		if (Tuple.IsValid())
		{
			Tuple->CancelHandle();
		}
	}

	for (auto Iterator = AttributeSetHandles.CreateIterator(); Iterator; ++Iterator)
	{
		const TSharedPtr<FStreamableHandle>& Tuple = Iterator->Value;
		if (Tuple.IsValid())
		{
			Tuple->CancelHandle();
		}
	}
	
	AbilityHandleSystem.Reset();
	AttributeSetHandles.Reset();
	AbilitySpecHandles.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Removing %s."),
	                *GetNameSafe(UAVVMAbilitySystemComponent::StaticClass()));

	OwningOuter.Reset();
}

void UAVVMAbilitySystemComponent::SetupAbilities(const TArray<UObject*>& Resources)
{
	const FGameplayTagContainer& OwnedTags = GetOwnedGameplayTags();

	const AActor* Outer = OwningOuter.Get();

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
			AVVM_LOGGER_LOG(LogGameplay,
			                Outer,
			                Outer,
			                TEXT("Failed to Grant Ability %s due to Blocking Tags."),
			                *GetNameSafe(AbilityAsset));

			continue;
		}
		
		DeferredGrantedAbilities.Add(AbilityAsset->GetGameplayAbilityClass().ToSoftObjectPath());
		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("New Ability Recorded %s for deferred Granting."),
		                *GetNameSafe(AbilityAsset));
	}

	if (DeferredGrantedAbilities.IsEmpty())
	{
		return;
	}

#if WITH_AUTOMATION_TESTS
	UAVVMAutomatedTestResourceValidationManager::Static_IncrementUObjectRequested(GetWorld(), this, DeferredGrantedAbilities.Num());
#endif
	
	const auto Token = FAbilityToken::MakeToken();
		
	FStreamableDelegate Callback;
	Callback.BindUObject(this, &UAVVMAbilitySystemComponent::OnAbilityGrantingDeferred, Token);

	TSharedPtr<FStreamableHandle>& OutResult = AbilityHandleSystem.FindOrAdd(Token.UniqueId);
	OutResult = UAssetManager::Get().LoadAssetList(DeferredGrantedAbilities, Callback);
}

void UAVVMAbilitySystemComponent::SetupAttributeSet(const FSoftObjectPath& AttributeSetSoftObjectPath,
                                                    AActor* AttributeSetOwner)
{
	const auto OnAsyncRequestComplete = [](const TWeakObjectPtr<UAVVMAbilitySystemComponent>& Caller,
	                                       const TWeakObjectPtr<AActor>& NewAttributeSetOwner)
	{
		AActor* AttributeSetOwner = NewAttributeSetOwner.Get();
		if (!IsValid(AttributeSetOwner))
		{
			return;
		}
		
		UAVVMAbilitySystemComponent* ASC = Caller.Get();
		if (!IsValid(ASC))
		{
			return;
		}

		TSharedPtr<FStreamableHandle>* StreamableHandle = ASC->AttributeSetHandles.Find(NewAttributeSetOwner);
		if (StreamableHandle == nullptr || !StreamableHandle->IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		(*StreamableHandle)->GetLoadedAssets(OutResources);

#if WITH_AUTOMATION_TESTS
		UAVVMAutomatedTestResourceValidationManager::Static_IncrementUObjectLoaded(AttributeSetOwner->GetWorld(), Caller.Get(), OutResources.Num());
#endif

		for (UObject* Resource : OutResources)
		{
			auto* AttributeSetClass = Cast<UClass>(Resource);
			if (!IsValid(AttributeSetClass))
			{
				continue;
			}

			const auto* AttributeSet = Cast<UAVVMAttributeSet>(ASC->GetOrCreateAttributeSubobject(AttributeSetClass));
			if (IsValid(AttributeSet))
			{
				Caller->RegisterAttributeSet(AttributeSet, AttributeSetOwner);
				const_cast<UAVVMAttributeSet*>(AttributeSet)->Init();
			}
		}
	};

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (AttributeSetSoftObjectPath.IsNull())
	{
		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("Attempt to load invalid AttributeSet."));

		return;
	}

#if WITH_AUTOMATION_TESTS
	UAVVMAutomatedTestResourceValidationManager::Static_IncrementUObjectRequested(GetWorld(), this);
#endif

	FStreamableDelegate Callback;
	Callback.BindWeakLambda(this, OnAsyncRequestComplete, TWeakObjectPtr(this), TWeakObjectPtr(AttributeSetOwner));

	TSharedPtr<FStreamableHandle>& OutResult = AttributeSetHandles.FindOrAdd(AttributeSetOwner);
	OutResult = UAssetManager::Get().LoadAssetList({AttributeSetSoftObjectPath}, Callback);
}

void UAVVMAbilitySystemComponent::RegisterAttributeSet(const UAttributeSet* AttributeSet, AActor* AttributeSetOwner)
{
	const bool bResult = UAVVMToolkitUtils::IsNativeScriptInterfaceValid<IAVVMDoesOwnAttributeSet>(AttributeSetOwner);
	if (!ensureAlwaysMsgf(bResult, TEXT("AttributeSetOwner should inherit from the IAVVMDoesOwnAttributeSet interface.")))
	{
		return;
	}

	// @gdemers register AttributeSet with 'this' ASC.
	const TWeakObjectPtr<const UAttributeSet>& OutResult = OwnerToAttributeSet.FindOrAdd(AttributeSetOwner, AttributeSet);
	AddSpawnedAttribute(const_cast<UAttributeSet*>(OutResult.Get()));

	// @gdemers cache AttributeSet on Owner for possible runtime swap. (Stored as TObjectPtr to prevent gc on the Outer)
	IAVVMDoesOwnAttributeSet::Execute_SetAttributeSet(AttributeSetOwner, AttributeSet);
}

void UAVVMAbilitySystemComponent::UnRegisterAttributeSet(const AActor* AttributeSetOwner)
{
	TWeakObjectPtr<const UAttributeSet> OutResult = nullptr;
	const bool bDoesExist = OwnerToAttributeSet.RemoveAndCopyValue(AttributeSetOwner, OutResult);
	if (!bDoesExist)
	{
		return;
	}

	TSharedPtr<FStreamableHandle>& StreamableHandle = AttributeSetHandles[AttributeSetOwner];
	if (StreamableHandle.IsValid())
	{
		StreamableHandle->ReleaseHandle();
	}

	OwnerToAttributeSet.Remove(AttributeSetOwner);
	AttributeSetHandles.Remove(AttributeSetOwner);
	RemoveSpawnedAttribute(const_cast<UAttributeSet*>(OutResult.Get()));
}

void UAVVMAbilitySystemComponent::OnAbilityGrantingDeferred(FAbilityToken AbilityToken)
{
	const TSharedPtr<FStreamableHandle>* OutResult = AbilityHandleSystem.Find(AbilityToken.UniqueId);
	if (!ensureAlwaysMsgf(OutResult != nullptr && OutResult->IsValid(), TEXT("Callback executed during destruction.")))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TArray<UObject*> OutStreamedAssets;
	(*OutResult)->GetLoadedAssets(OutStreamedAssets);

#if WITH_AUTOMATION_TESTS
	UAVVMAutomatedTestResourceValidationManager::Static_IncrementUObjectLoaded(GetWorld(), this, OutStreamedAssets.Num());
#endif

	for (UObject* StreamableAsset : OutStreamedAssets)
	{
		auto* GameplayAbilityClass = Cast<UClass>(StreamableAsset);
		if (!IsValid(GameplayAbilityClass))
		{
			continue;
		}

		AVVM_LOGGER_LOG(LogGameplay,
		                Outer,
		                Outer,
		                TEXT("Granting New Ability %s."),
		                *GetNameSafe(GameplayAbilityClass));

		const auto GameplayAbilitySpec = FGameplayAbilitySpec
		{
				GameplayAbilityClass,
				1,
				GameplayAbilityClass->GetDefaultObject<UAVVMGameplayAbility>()->GetInputId()
		};

		const FGameplayAbilitySpecHandle NewAbilitySpecHandle = GiveAbility(GameplayAbilitySpec);
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

	AVVM_LOGGER_LOG(LogGameplay,
	                Outer,
	                Outer,
	                TEXT("Modifying Tag %s."),
	                *Tag.ToString());
}
