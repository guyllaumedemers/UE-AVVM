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
#include "AVVMScopedDelegate.h"
#include "TriggeringAttachmentActor.h"
#include "WeaponSample.h"
#include "Data/AttachmentDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

void UAttachmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BatchingMechanism = MakeShared<FAttachmentBatchingMechanism>();
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

	BatchingMechanism.Reset();
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
	TWeakObjectPtr<ATriggeringAttachmentActor>& SearchResult = EquippedAttachments.FindOrAdd(NewAttachmentSwapContext.TargetSlotTag);
	if (SearchResult.IsValid())
	{
		// @gdemers detach old actor from parent.
		SearchResult->Detach();
	}

	if (BatchingMechanism.IsValid())
	{
		// @gdemers add old reference to batch destroy collection.
		BatchingMechanism->PushPendingDestroy(SearchResult.Get());
	}

	// @gdemers update entry reference.
	SearchResult = NewAttachmentSwapContext.Attachment;
	if (SearchResult.IsValid())
	{
		// @gdemers attach new actor to parent.
		SearchResult->Attach();
	}
}

void UAttachmentManagerComponent::GetAttachmentModifierDefinition(const FDataRegistryId& NewAttachmentModifierDefinitionId) const
{
	auto* ResourceManagerComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(OwningOuter.Get());
	if (!ensureAlwaysMsgf(IsValid(ResourceManagerComponent), TEXT("Outer doesn't return valid AVVMResourceManagerComponent!")))
	{
		ResourceManagerComponent->RequestAsyncLoading(NewAttachmentModifierDefinitionId, {});
	}
}

void UAttachmentManagerComponent::SetupAttachments(const TArray<UObject*>& NewResources)
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

		DeferredItems.Add(AttachmentAsset->GetTriggeringAttachmentClass().ToSoftObjectPath());
	}

	if (!DeferredItems.IsEmpty())
	{
		FAttachmentToken Token;
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAttachmentManagerComponent::OnAttachmentActorRetrieved, Token);

		TSharedPtr<FStreamableHandle>& OutResult = AttachmentHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
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
	                      TEXT("Attempting to load invalid Attachment Modifier set on Outer \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<FSoftObjectPath> DeferredItems;
	for (const UObject* Resource : NewResources)
	{
		const auto* AttachmentModifierAsset = Cast<UAttachmentModifierDefinitionDataAsset>(Resource);
		if (!IsValid(AttachmentModifierAsset))
		{
			continue;
		}

		UE_LOG(LogWeaponSample,
		       Log,
		       TEXT("Executed from \"%s\". New \"%s\" Recorded."),
		       IsServerOrClientString,
		       *AttachmentModifierAsset->GetName());

		DeferredItems.Append(AttachmentModifierAsset->GetModifiersSoftObjectPaths());
	}

	if (!DeferredItems.IsEmpty())
	{
		FAttachmentModifierToken Token;
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAttachmentManagerComponent::OnAttachmentModifiersRetrieved, Token);

		TSharedPtr<FStreamableHandle>& OutResult = AttachmentModifierHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
	}
}

void UAttachmentManagerComponent::OnAttachmentActorRetrieved(FAttachmentToken AttachmentToken)
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

	TArray<UObject*> OutStreamableAssets;
	(*OutResult)->GetLoadedAssets(OutStreamableAssets);

	if (OutStreamableAssets.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = const_cast<AActor*>(Outer);

	auto* NewAttachment = Cast<ATriggeringAttachmentActor>(World->SpawnActor(Cast<UClass>(OutStreamableAssets[0]), &FTransform::Identity, Params));
	if (!ensureAlwaysMsgf(IsValid(NewAttachment), TEXT("Attachment Actor Class Failed to create an instance in World!")))
	{
		return;
	}

	Swap(FAttachmentSwapContextArgs{NewAttachment, NewAttachment->SlotTag});

	if (!QueueingMechanism.IsValid())
	{
		return;
	}

	const bool bIsEmpty = QueueingMechanism->IsEmpty();

	// @gdemers we should always push in order to queue subsequent request.
	QueueingMechanism->Push(NewAttachment);

	if (bIsEmpty)
	{
		// @gdemers async load attachment modifier for the actor that is first spawned in world.
		GetAttachmentModifierDefinition(NewAttachment->GetAttachmentModifierDefinitionId());
	}
}

void UAttachmentManagerComponent::OnAttachmentModifiersRetrieved(FAttachmentModifierToken AttachmentModifierToken)
{
	// @gdemers our attachment is fully initialized, and we can move to the next request.
	const auto OnNextRequestExecuted = [](const TSharedPtr<FAttachmentQueuingMechanism> NewQueueingMechanism,
	                                      const UAttachmentManagerComponent* Component)
	{
		if (NewQueueingMechanism.IsValid())
		{
			NewQueueingMechanism->TryExecuteNext(Component);
		}
	};

	// @gdemers for safety reason, our callback is using a scoped object.
	const auto Callback = TDelegate<void()>::CreateWeakLambda(this, OnNextRequestExecuted, QueueingMechanism, this);
	FAVVMScopedDelegate ScopedDelegate(Callback);

	const TSharedPtr<FStreamableHandle>* OutResult = AttachmentModifierHandleSystem.Find(AttachmentModifierToken.UniqueId);
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	TWeakObjectPtr<ATriggeringAttachmentActor> AttachmentActor = QueueingMechanism.IsValid() ? QueueingMechanism->PeekAtIndex(0) : nullptr;
	if (!AttachmentActor.IsValid())
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	(*OutResult)->GetLoadedAssets(OutStreamableAssets);

	// @gdemers our Outer actor refers to the TriggeringActor which doesn't hold an ASC. It is expected that the user adds an ASC
	// to a Controller when using this system with AI and on the PlayerState for local player behaviour. 
	auto* Controller = GetTypedOuter<AController>();
	if (ensureAlwaysMsgf(IsValid(Controller), TEXT("GetTypedOuter doesn't refer to any Controller derived type!")))
	{
		auto* ASCHolder = IsValid(Controller->PlayerState) ? Cast<AActor>(Controller->PlayerState) : Cast<AActor>(Controller);
		auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ASCHolder);
		AttachmentActor->RegisterGameplayEffects(ASC, OutStreamableAssets);
	}
}

UAttachmentManagerComponent::FAttachmentQueuingMechanism::~FAttachmentQueuingMechanism()
{
	QueuedRequest.Empty();
}

void UAttachmentManagerComponent::FAttachmentQueuingMechanism::Push(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewAttachment)
{
	QueuedRequest.Add(NewAttachment);
}

TWeakObjectPtr<ATriggeringAttachmentActor> UAttachmentManagerComponent::FAttachmentQueuingMechanism::PeekAtIndex(const int32 NewIndex)
{
	return QueuedRequest.IsValidIndex(NewIndex) ? QueuedRequest[NewIndex] : nullptr;
}

void UAttachmentManagerComponent::FAttachmentQueuingMechanism::TryExecuteNext(const UAttachmentManagerComponent* AttachmentManagerComponent)
{
	if (!IsValid(AttachmentManagerComponent))
	{
		return;
	}

	if (!QueuedRequest.IsEmpty())
	{
		QueuedRequest.RemoveAtSwap(0);
	}

	if (!QueuedRequest.IsEmpty())
	{
		const FDataRegistryId& AttachmentModifierRegistryId = QueuedRequest[0]->GetAttachmentModifierDefinitionId();
		AttachmentManagerComponent->GetAttachmentModifierDefinition(AttachmentModifierRegistryId);
	}
}

bool UAttachmentManagerComponent::FAttachmentQueuingMechanism::IsEmpty() const
{
	return QueuedRequest.IsEmpty();
}

UAttachmentManagerComponent::FAttachmentBatchingMechanism::~FAttachmentBatchingMechanism()
{
	PendingDestroy.Reset();
}

void UAttachmentManagerComponent::FAttachmentBatchingMechanism::PushPendingDestroy(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewAttachment)
{
	if (NewAttachment.IsValid())
	{
		PendingDestroy.Add(NewAttachment);
	}
}

void UAttachmentManagerComponent::FAttachmentBatchingMechanism::BatchDestroy()
{
	for (auto Iterator = PendingDestroy.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			Iterator->Get()->Destroy();
			Iterator.RemoveCurrentSwap();
		}
	}
}
