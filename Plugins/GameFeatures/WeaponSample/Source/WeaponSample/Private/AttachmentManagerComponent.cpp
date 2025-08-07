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
#include "AttachmentManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "AVVMGameplayUtils.h"
#include "TriggeringAttachmentActor.h"
#include "WeaponSample.h"
#include "Data/AttachmentDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

void UAttachmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	QueueingMechanism = MakeShared<FAttachmentQueuingMechanism>();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	OwningOuter = Outer;

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAttachmentManagerComponent::StaticClass()->GetName(),
	       *Outer->GetName());
}

void UAttachmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	QueueingMechanism.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAttachmentManagerComponent::StaticClass()->GetName(),
	       *Outer->GetName());
}

void UAttachmentManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UAttachmentManagerComponent* UAttachmentManagerComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UAttachmentManagerComponent>() : nullptr;
}

void UAttachmentManagerComponent::Swap_Implementation(const FAttachmentSwapContextArgs& NewAttachmentSwapContext)
{
	// TODO @gdemers Handle slot swapping. Removing a Slot without reassigning should remove
	// all GE from the owning attachment.
}

void UAttachmentManagerComponent::GetAttachmentDefinition(const FGetAttachmentDefinitionRequestArgs& NewRequestArgs)
{
	if (QueueingMechanism.IsValid())
	{
		QueueingMechanism->Push(FAttachmentStreamableContext{NewRequestArgs.AttachmentActor.Get()});
	}

	auto* ResourceComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(OwningOuter.Get());
	if (ensureAlwaysMsgf(IsValid(ResourceComponent),
	                     TEXT("Outer is missing IAVVMResourceProvider::GetResourceManagerComponent impl or return nullptr.")))
	{
		ResourceComponent->RequestAsyncLoading(NewRequestArgs.AttachmentId, {});
	}
}

void UAttachmentManagerComponent::SetupAttachmentModifiers(const TArray<UObject*>& NewResources)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                      TEXT("Attempting to load invalid Attachment set on Outer \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<FSoftObjectPath> DeferredItems;
	for (const UObject* Resource : NewResources)
	{
		const auto* AttachmentAsset = Cast<UAttachmentDefinitionDataAsset>(Resource);
		if (!IsValid(AttachmentAsset))
		{
			continue;
		}

		if (!AttachmentAsset->CanAccessItem(ComponentStateTags, ComponentStateTags))
		{
			UE_LOG(LogWeaponSample,
			       Log,
			       TEXT("Executed from \"%s\". Failed to Meet \"%s\" Requirements."),
			       IsServerOrClientString,
			       *AttachmentAsset->GetName());

			continue;
		}

		UE_LOG(LogWeaponSample,
		       Log,
		       TEXT("Executed from \"%s\". New \"%s\" Recorded."),
		       IsServerOrClientString,
		       *AttachmentAsset->GetName());

		DeferredItems.Append(AttachmentAsset->GetModifiersSoftObjectPaths());
	}

	if (!DeferredItems.IsEmpty())
	{
		FAttachmentToken Token;
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAttachmentManagerComponent::OnAttachmentModifiersRetrieved, Token);

		// TODO @gdemers ordering should be garantied from the AVVMResourceManagerComponent but still double check the possibility of a racing conditions.
		// I remember fixing issues on the resource manager system for this kind of cases.
		if (QueueingMechanism.IsValid())
		{
			QueueingMechanism->ModifyIndex(Token.UniqueId);
		}

		TSharedPtr<FStreamableHandle>& OutResult = AttachmentHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
	}
}

void UAttachmentManagerComponent::OnAttachmentModifiersRetrieved(FAttachmentToken AttachmentToken)
{
	const TSharedPtr<FStreamableHandle>* OutResult = AttachmentHandleSystem.Find(AttachmentToken.UniqueId);
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	TWeakObjectPtr<ATriggeringAttachmentActor> AttachmentActor = QueueingMechanism->PeekAtIndex(AttachmentToken.UniqueId);
	if (AttachmentActor.IsValid())
	{
		TArray<UObject*> OutStreamableAssets;
		(*OutResult)->GetLoadedAssets(OutStreamableAssets);

		// TODO @gdemers Outer isnt the player state that holds the triggerable actor
		// but the triggerable actor itself. maybe a recursive search of parent impose. TBD!
		auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Outer);
		AttachmentActor->RegisterGameplayEffects(ASC, OutStreamableAssets);
	}
}

UAttachmentManagerComponent::FAttachmentStreamableContext::FAttachmentStreamableContext(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewAttachment)
	: StreamableContextId(INDEX_NONE)
	, Attachment(NewAttachment)
{
}

UAttachmentManagerComponent::FAttachmentQueuingMechanism::~FAttachmentQueuingMechanism()
{
	QueuedRequest.Empty();
}

void UAttachmentManagerComponent::FAttachmentQueuingMechanism::Push(const FAttachmentStreamableContext& NewContext)
{
	QueuedRequest.Add(MakeShared<FAttachmentStreamableContext>(NewContext));
}

TWeakObjectPtr<ATriggeringAttachmentActor> UAttachmentManagerComponent::FAttachmentQueuingMechanism::PeekAtIndex(const int32 NewIndex)
{
	const TSharedPtr<FAttachmentStreamableContext>* SearchResult = QueuedRequest.FindByPredicate([SearchIndex = NewIndex](TSharedPtr<FAttachmentStreamableContext> NewContext)
	{
		return NewContext.IsValid() && NewContext->StreamableContextId == SearchIndex;
	});

	TWeakObjectPtr<ATriggeringAttachmentActor> Out;
	if (SearchResult != nullptr && SearchResult->IsValid())
	{
		Out = (*SearchResult)->Attachment;
	}

	return Out;
}

void UAttachmentManagerComponent::FAttachmentQueuingMechanism::ModifyIndex(const int32 NewIndex)
{
	const TSharedPtr<FAttachmentStreamableContext>* SearchResult = QueuedRequest.FindByPredicate([](TSharedPtr<FAttachmentStreamableContext> NewContext)
	{
		return NewContext.IsValid() && NewContext->StreamableContextId == INDEX_NONE;
	});

	if (SearchResult != nullptr && SearchResult->IsValid())
	{
		(*SearchResult)->StreamableContextId = NewIndex;
	}
}
